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
}
