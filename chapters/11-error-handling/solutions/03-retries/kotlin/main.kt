const val MAX_ATTEMPTS = 3

// The base class, sealed so that no other file can add a kind.
sealed class DownloadException(message: String, cause: Throwable? = null) :
    Exception(message, cause) {
    // Try the same file again?
    val retry: Boolean
        get() = this is TimedOut || this is ServerBusy
}

class TimedOut : DownloadException("timed out")

class ServerBusy : DownloadException("server busy")

class NotFound : DownloadException("not found")

class LoginRefused : DownloadException("login refused")

class GaveUp(attempts: Int, last: DownloadException) :
    DownloadException("gave up after $attempts attempts: ${last.message}", last)

// The pretend server: throw the error for this attempt's answer, if any.
fun download(answers: List<String>, attempt: Int) {
    when (answers[minOf(attempt, answers.size) - 1]) {
        "timeout" -> throw TimedOut()
        "busy" -> throw ServerBusy()
        "missing" -> throw NotFound()
        "auth" -> throw LoginRefused()
    }
}

// Return the number of attempts it took, or throw the error that stopped it.
fun downloadWithRetries(name: String, answers: List<String>): Int {
    var wait = 1
    for (attempt in 1..MAX_ATTEMPTS) {
        try {
            download(answers, attempt)
            return attempt
        } catch (e: DownloadException) {
            if (!e.retry) throw e
            if (attempt == MAX_ATTEMPTS) throw GaveUp(attempt, e)
            println("$name: ${e.message}, retrying in ${wait}s")
            wait *= 2
        }
    }
    error("the last attempt always returns or throws")
}

fun main() {
    val jobs =
        generateSequence(::readlnOrNull)
            .map { it.trim().split(Regex(" +")) }
            .filter { it.first().isNotEmpty() }
            .toList()

    var downloaded = 0
    var skipped = 0
    var failed = 0
    var notTried = 0
    for ((index, job) in jobs.withIndex()) {
        val name = job.first()
        try {
            val attempts = downloadWithRetries(name, job.drop(1))
            println("$name: ok after $attempts attempt${if (attempts == 1) "" else "s"}")
            downloaded++
        } catch (e: DownloadException) {
            when (e) {
                is GaveUp -> {
                    println("$name: failed: ${e.message}")
                    failed++
                }
                is LoginRefused -> {
                    println("$name: stopped: ${e.message}")
                    failed++
                    notTried = jobs.size - index - 1
                    break
                }
                else -> {
                    println("$name: skipped: ${e.message}")
                    skipped++
                }
            }
        }
    }
    println("downloaded $downloaded, skipped $skipped, failed $failed, not tried $notTried")
}
