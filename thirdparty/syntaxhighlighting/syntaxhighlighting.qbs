import qbs

Project {
    id: syntaxhighlighting
    property stringList srcFiles: [
        "src/*.cpp",
        "src/*.h",
        "themes/*.qrc",
        "syntax/*.qrc"
    ]

    property stringList incPaths: [
        "src"
    ]

    StaticLibrary {
        name: "syntaxhighlighting"
        condition: syntaxhighlighting.desktop
        files: syntaxhighlighting.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "Qt"; submodules: ["core", "gui", "network"]; }
        bundle.isBundle: false

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.linkerVariant: syntaxhighlighting.linkerVariant
        }

        cpp.defines: [ "HAS_SYNTAX_RESOURCE", "SyntaxHighlighting_VERSION_MAJOR=5", "SyntaxHighlighting_VERSION_MINOR=57" ]
        cpp.includePaths: syntaxhighlighting.incPaths
    }
}
