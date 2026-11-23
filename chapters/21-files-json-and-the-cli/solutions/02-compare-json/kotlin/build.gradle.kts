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
}
