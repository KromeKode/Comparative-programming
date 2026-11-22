// The build has two projects: this folder's, the program, and textstats.
rootProject.name = "main"
include("textstats")

// Where Gradle finds its plugins and the libraries they need.
pluginManagement {
    repositories {
        gradlePluginPortal()
    }
}
dependencyResolutionManagement {
    repositories {
        mavenCentral()
    }
}
