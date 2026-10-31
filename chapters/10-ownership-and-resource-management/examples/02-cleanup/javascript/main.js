// A pretend file that prints when it opens and closes.
class File {
  constructor(name, fails = false) {
    if (fails) throw new Error(`cannot open ${name}`);
    console.log(`  open ${name}`);
    this.name = name;
  }

  close() {
    console.log(`  close ${this.name}`);
  }
}

// A finally block runs however its try block ends, so each file gets one.
function runJob(outputFails, copyFails) {
  const input = new File("input");
  try {
    const output = new File("output", outputFails);
    try {
      if (copyFails) throw new Error("disk full");
      console.log("  copy input to output");
    } finally {
      output.close();
    }
  } finally {
    input.close();
  }
}

const jobs = [
  ["everything works", false, false],
  ["opening output fails", true, false],
  ["copying fails", false, true],
];
for (const [i, [title, outputFails, copyFails]] of jobs.entries()) {
  const number = i + 1;
  console.log(`job ${number}: ${title}`);
  try {
    runJob(outputFails, copyFails);
    console.log(`  job ${number} done`);
  } catch (error) {
    console.log(`  job ${number} failed: ${error.message}`);
  }
}
