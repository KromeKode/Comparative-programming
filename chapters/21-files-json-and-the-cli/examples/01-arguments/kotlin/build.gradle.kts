plugins {
    kotlin("jvm") version "2.4.21"
    application
}

dependencies {
    // Clikt reads the command line: options, arguments, and --help.
    implementation("com.github.ajalt.clikt:clikt:5.0.3")
}

kotlin {
    compilerOptions {
        allWarningsAsErrors = true
    }
}

application {
    mainClass = "MainKt"
    // The JVM writes stdout in the locale's character set, which is ASCII when LANG is
    // unset or C, and then writes "é" as "?". These make it UTF-8 whatever the locale.
    applicationDefaultJvmArgs = listOf("-Dstdout.encoding=UTF-8", "-Dstderr.encoding=UTF-8")
}
