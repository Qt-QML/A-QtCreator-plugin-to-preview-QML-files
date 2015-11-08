import qbs 1.0
import qbs.FileInfo

QtcPlugin {
    name: "QmlPreview"

    Depends { name: "Qt"; submodules: ["widgets", "quick", "quickwidgets"] }
    Depends { name: "Core" }
    Depends { name: "QmlJSEditor" }
    Depends { name: "QmlJSTools" }
    Depends { name: "TextEditor" }
    Depends { name: "Utils" }

    cpp.cxxFlags: "-std=c++11"
    cpp.cxxLanguageVersion: "c++11"

    files: [
        "previewsfiltermodel.cpp",
        "previewsfiltermodel.h",
        "qmlpreviewconstants.h",
        "qmlpreviewplugin.cpp",
        "qmlpreviewplugin.h",
        "widgets/previewwidget.cpp",
        "widgets/previewwidget.h",
    ]

    Group {
        name: "Tests"
        condition: project.testsEnabled

        files: [
        ]

        cpp.defines: outer.concat(['SRCDIR="' + FileInfo.path(filePath) + '"'])
    }
}
