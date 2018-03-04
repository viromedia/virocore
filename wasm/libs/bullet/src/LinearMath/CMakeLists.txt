
include_directories(
	../../include
)

set(LinearMath_SRCS
	btAlignedAllocator.cpp
	btConvexHull.cpp
	btConvexHullComputer.cpp
	btGeometryUtil.cpp
	btPolarDecomposition.cpp
	btQuickprof.cpp
	btSerializer.cpp
	btVector3.cpp
)

set(LinearMath_HDRS
	btAabbUtil2.h
	btAlignedAllocator.h
	btAlignedObjectArray.h
	btConvexHull.h
	btConvexHullComputer.h
	btDefaultMotionState.h
	btGeometryUtil.h
	btGrahamScan2dConvexHull.h
	btHashMap.h
	btIDebugDraw.h
	btList.h
	btMatrix3x3.h
	btMinMax.h
	btMotionState.h
	btPolarDecomposition.h
	btPoolAllocator.h
	btQuadWord.h
	btQuaternion.h
	btQuickprof.h
	btRandom.h
	btScalar.h
	btSerializer.h
	btStackAlloc.h
	btTransform.h
	btTransformUtil.h
	btVector3.h
)

add_library(bullet_linearmath ${LinearMath_SRCS} ${LinearMath_HDRS})
