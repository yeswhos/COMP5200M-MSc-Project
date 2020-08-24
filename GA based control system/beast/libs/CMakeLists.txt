

if( NOT WIN32 )
  SET(LIB_TYPE SHARED)
else( NOT WIN32 )
  SET(LIB_TYPE STATIC)
endif( NOT WIN32 )

set( beast_HEADERS
     animat.h
     animatmonitor.h
     bacteria.h
     bacterium.h
     beast.h
     collisions.h
     colours.h
     distribution.h
     drawable.h
     dynamicalnet.h
     feedforwardnet.h
     geneticalgorithm.h
     glutsimenv.h
     neuralanimat.h
     population.h
     psoalgorithm.h
     random.h
     sensorbase.h
     sensorfunctors.h
     sensor.h
     serialfuncs.h
     signaller.h
     simulation.h
     trail.h
     unserialiser.h
     utilities.h
     vector2d.h
     vector3d.h
     world.h
     worldobject.h
)
    
set( beast_SRC
     animat.cc
     animatmonitor.cc
     bacteria.cc
     bacterium.cc
     beast.cc
     collisions.cc
     distribution.cc
     drawable.cc
     dynamicalnet.cc
     feedforwardnet.cc
     glut.cc
     neuralanimat.cc
     sensor.cc
     serialfuncs.cc
     simulation.cc
     trail.cc
     unserialiser.cc
     world.cc
     worldobject.cc
)

add_library (BEASTLIB SHARED ${beat_HDR} ${beast_SRC})
