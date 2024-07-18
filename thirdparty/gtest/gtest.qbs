import qbs

Project {
    id: gtest
    property stringList srcFiles: [
        "src/gtest-all.cc",
        "src/gtest_main.cc"
    ]

    property stringList incPaths: [
        ".",
        "include"
    ]

    StaticLibrary {
		condition: gtest.desktop
        name: "gtest"
        files: gtest.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: gtest.incPaths

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.linkerVariant: gtest.linkerVariant
        }
    }
}
