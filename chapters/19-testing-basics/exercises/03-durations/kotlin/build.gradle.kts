plugins {
    kotlin("jvm") version "2.4.21"
}

dependencies {
    // kotlin.test's assertions, and its @Test, which runs on JUnit 5.
    testImplementation(kotlin("test"))
    // @ParameterizedTest, from JUnit itself, at the version kotlin.test uses.
    testImplementation("org.junit.jupiter:junit-jupiter-params")
}

kotlin {
    compilerOptions {
        allWarningsAsErrors = true
    }
}

tasks.test {
    // Run the tests on the JUnit Platform, which is how JUnit 5 finds and runs them.
    useJUnitPlatform()
    // Print each failing test in full, with its message and not only its class, even
    // with --quiet, which the checker runs Gradle with.
    testLogging.quiet {
        events("failed")
        exceptionFormat = org.gradle.api.tasks.testing.logging.TestExceptionFormat.FULL
    }
}
