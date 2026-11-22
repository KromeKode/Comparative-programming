plugins {
    kotlin("jvm") version "2.4.21"
    // application adds tasks to run the program and to package it with a script.
    application
}

dependencies {
    // A dependency on the other project of this build, not on a published library.
    implementation(project(":textstats"))
}

kotlin {
    compilerOptions {
        allWarningsAsErrors = true
    }
}

application {
    // Top-level functions in main.kt compile into a class called MainKt.
    mainClass = "MainKt"
}
