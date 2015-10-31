import qbs 1.0
import qbs.FileInfo

QtcPlugin {
    name: "QmlPreview"

    Depends { name: "Qt"; submodules: ["widgets"] }
    Depends { name: "Core" }

    cpp.cxxFlags: "-std=c++11"
    cpp.cxxLanguageVersion: "c++11"

    files: [
        "qmlpreviewplugin.cpp",
        "qmlpreviewplugin.h",
    ]

    Group {
        name: "Tests"
        condition: project.testsEnabled

        files: [
        ]

        cpp.defines: outer.concat(['SRCDIR="' + FileInfo.path(filePath) + '"'])
    }
}
