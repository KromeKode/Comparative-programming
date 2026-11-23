plugins {
    kotlin("jvm") version "2.4.21"
    // The serialization plugin writes the code that turns each @Serializable class into
    // JSON and back, when the program is compiled.
    kotlin("plugin.serialization") version "2.4.21"
    application
}

dependencies {
    implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.9.0")
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
