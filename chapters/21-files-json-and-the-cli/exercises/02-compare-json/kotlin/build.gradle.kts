plugins {
    kotlin("jvm") version "2.4.21"
    application
}

dependencies {
    // Only the library: the serialization plugin writes code for @Serializable classes,
    // and this program reads JSON into JsonElements instead.
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
