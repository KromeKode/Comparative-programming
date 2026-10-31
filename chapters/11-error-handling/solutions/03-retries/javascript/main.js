import { readFileSync } from "node:fs";

const MAX_ATTEMPTS = 3;

// The base class. Each kind says whether to try again or stop everything.
class DownloadError extends Error {
  retry = false;
  fatal = false;
}

class TimedOut extends DownloadError {
  retry = true;
  constructor() {
    super("timed out");
  }
}

class ServerBusy extends DownloadError {
  retry = true;
  constructor() {
    super("server busy");
  }
}

class NotFound extends DownloadError {
  constructor() {
    super("not found");
  }
}

class LoginRefused extends DownloadError {
  fatal = true;
  constructor() {
    super("login refused");
  }
}

class GaveUp extends DownloadError {
  constructor(attempts, last) {
    super(`gave up after ${attempts} attempts: ${last.message}`, {
      cause: last,
    });
  }
}

const ERRORS = new Map([
  ["timeout", TimedOut],
  ["busy", ServerBusy],
  ["missing", NotFound],
  ["auth", LoginRefused],
]);

// The pretend server: throw the error for this attempt's answer, if any.
function download(answers, attempt) {
  const answer = answers[Math.min(attempt, answers.length) - 1];
  const ErrorType = ERRORS.get(answer);
  if (ErrorType !== undefined) {
    throw new ErrorType();
  }
}

// Return the number of attempts it took, or throw the error that stopped it.
function downloadWithRetries(name, answers) {
  let wait = 1;
  for (let attempt = 1; ; attempt++) {
    try {
      download(answers, attempt);
      return attempt;
    } catch (error) {
      if (!(error instanceof DownloadError) || !error.retry) {
        throw error;
      }
      if (attempt === MAX_ATTEMPTS) {
        throw new GaveUp(attempt, error);
      }
      console.log(`${name}: ${error.message}, retrying in ${wait}s`);
      wait *= 2;
    }
  }
}

const jobs = readFileSync(0, "utf8")
  .split("\n")
  .map((line) => line.trim().split(/ +/))
  .filter((words) => words[0] !== "");

let [downloaded, skipped, failed, notTried] = [0, 0, 0, 0];
for (const [index, [name, ...answers]] of jobs.entries()) {
  try {
    const attempts = downloadWithRetries(name, answers);
    const plural = attempts === 1 ? "" : "s";
    console.log(`${name}: ok after ${attempts} attempt${plural}`);
    downloaded++;
  } catch (error) {
    if (!(error instanceof DownloadError)) {
      throw error;
    }
    if (error instanceof GaveUp) {
      console.log(`${name}: failed: ${error.message}`);
      failed++;
    } else if (error.fatal) {
      console.log(`${name}: stopped: ${error.message}`);
      failed++;
      notTried = jobs.length - index - 1;
      break;
    } else {
      console.log(`${name}: skipped: ${error.message}`);
      skipped++;
    }
  }
}
console.log(
  `downloaded ${downloaded}, skipped ${skipped}, failed ${failed}, not tried ${notTried}`,
);
