
project( ${BEAST_LIBRARY_PREFIX}beast )

if( NOT WIN32 )
  SET(LIB_TYPE SHARED)
else( NOT WIN32 )
  SET(LIB_TYPE STATIC)
endif( NOT WIN32 )

set( beast_SRC
    DinA.cpp
    GeneralException.cpp
    Line.cpp
    LineAttributes.cpp
    LogStream.cpp
    Page.cpp
    PositionInCm.cpp
    RGBValue.cpp
    Rect.cpp
    RectAttributes.cpp
    Streamable.cpp
    Timer.cpp
    Util.cpp
    UtilLibException.cpp )
    
set( beast_HEADERS
    AbstractException.h
    BasicDefinitions.h
    ColorTable.h
    ConcreteStreamable.h
    DinA.h
    FailAlloc.h
    GeneralException.h
    IsFinite.h
    IsNan.h
    Line.h
    LineAttributes.h
    LocalDefinitions.h
    LogStream.h
    MToCm.h
    NumericException.h
    Page.h
    PaperFormatError.h
    PositionInCm.h
    Rect.h
    RectAttributes.h
    RGBValue.h
    Shape.h
    SizeInCm.h
    Streamable.h
    TimeException.h
    Timer.h
    ToValarray.h
    ToVector.h
    UtilLibException.h
    UtilLib.h
    UtilTest.h )

add_library( ${BEAST_LIBRARY_PREFIX}util ${LIB_TYPE} ${beast_SRC} ${beast_HEADERS})

install( FILES ${beast_HEADERS}
    DESTINATION include/beast/beastlib )

install( TARGETS ${BEAST_LIBRARY_PREFIX}beast
    LIBRARY DESTINATION lib 
    ARCHIVE DESTINATION lib )
