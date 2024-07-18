import qbs

Project {
    id: pugixml
    property stringList srcFiles: [
        "src/**/*.cpp",
        "src/**/*.h"
    ]

    property stringList incPaths: [
        "src"
    ]

    StaticLibrary {
        name: "pugixml"
        files: pugixml.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.linkerVariant: pugixml.linkerVariant
        }

        cpp.includePaths: pugixml.incPaths
        cpp.cxxLanguageVersion: pugixml.languageVersion
        cpp.cxxStandardLibrary: pugixml.standardLibrary
        cpp.minimumMacosVersion: pugixml.osxVersion
    }
}
