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
}
