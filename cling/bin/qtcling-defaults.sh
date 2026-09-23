#!/bin/sh

# Edit this path before building qtcling to select the default Qt kit for this
# checkout. QTCLING_QT_ROOT still overrides it for an individual invocation.
QTCLING_DEFAULT_QT_ROOT=${QTCLING_DEFAULT_QT_ROOT:-/usr/local/qt/Qt/6.11.1/gcc_arm64}
