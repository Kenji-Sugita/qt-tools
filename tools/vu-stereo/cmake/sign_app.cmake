if(NOT DEFINED APP_BUNDLE)
    message(FATAL_ERROR "APP_BUNDLE is not set")
endif()

set(SIGN_IDENTITY "vu-stereo Local Code Signing")
set(ACTUAL_IDENTITY "-")

execute_process(
    COMMAND /usr/bin/security find-identity -v -p codesigning
    OUTPUT_VARIABLE CODESIGN_IDENTITIES
    ERROR_VARIABLE CODESIGN_IDENTITIES_ERROR
    RESULT_VARIABLE CODESIGN_IDENTITIES_RESULT
)

if(CODESIGN_IDENTITIES_RESULT EQUAL 0 AND CODESIGN_IDENTITIES MATCHES "${SIGN_IDENTITY}")
    set(ACTUAL_IDENTITY "${SIGN_IDENTITY}")
else()
    message(FATAL_ERROR "Code signing identity '${SIGN_IDENTITY}' was not visible to this build process. Build from a terminal where 'security find-identity -v -p codesigning' lists it.")
endif()

execute_process(
    COMMAND /usr/bin/codesign --force --deep --timestamp=none --sign "${ACTUAL_IDENTITY}" "${APP_BUNDLE}"
    RESULT_VARIABLE CODESIGN_RESULT
)

if(NOT CODESIGN_RESULT EQUAL 0)
    message(FATAL_ERROR "codesign failed with identity '${ACTUAL_IDENTITY}'")
endif()
