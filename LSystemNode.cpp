#include "LSystemNode.h"

#include <maya/MTime.h>
#include <maya/MFnMesh.h>
#include <maya/MPoint.h>
#include <maya/MFloatPoint.h>
#include <maya/MGlobal.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnStringData.h>
#include "LSystem.h"
#include "cylinder.h"

#define McheckErr(stat,msg)			\
	if ( MS::kSuccess != stat ) {	\
		cerr << msg;				\
		return MS::kFailure;		\
	}
MObject LSystemNode::time;
MObject LSystemNode::stepSize;
MObject LSystemNode::angle;
MObject LSystemNode::outputMesh;
MObject LSystemNode::grammar;
MObject LSystemNode::maxIterations;
MTypeId LSystemNode::id( 0x7ffff );

MStatus returnStatus;
LSystemNode::LSystemNode() : firstTime{ true }, currentError{ MStatus::kSuccess }  {}
LSystemNode::~LSystemNode() {} 
void* LSystemNode::creator()
{
	return new LSystemNode;
}

MStatus LSystemNode::initialize()
{
	MFnUnitAttribute unitAttr;
	MFnTypedAttribute typedAttr;
	MStatus returnStatus;
	MFnNumericAttribute angle;
	MFnStringData sdata;
	LSystemNode::time = unitAttr.create( "time", "tm", MFnUnitAttribute::kTime, 0.0, &returnStatus );
	McheckErr(returnStatus, "ERROR creating LSystemNode time attribute\n");
    LSystemNode::stepSize = angle.create("StepSize", "s", MFnNumericData::kFloat, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating LSystemNode stepSize attribute\n");
	LSystemNode::angle = angle.create("angle", "a", MFnNumericData::kFloat, 90, &returnStatus);
	McheckErr(returnStatus, "ERROR creating LSystemNode angle attribute\n");
	LSystemNode::maxIterations = angle.create("iterations", "i", MFnNumericData::kInt, 10, &returnStatus);
	myCheckError(&returnStatus, "ERROR creating LSystemNode iteration attribute\n");
	MObject strOBj = sdata.create("F+F+", &returnStatus);
	myCheckError(&returnStatus, "Create String Error");
	grammar = typedAttr.create("grammar", "g", MFnData::kString, strOBj, &returnStatus);
	myCheckError(&returnStatus, "string copied correctly");
	typedAttr.setStorable(true);
	MObject testStrObj = sdata.create("Hello", &returnStatus);
	myCheckError(&returnStatus, "string copied correctly");
//	testString = typedAttr.create("testString", "ts", 
//					MFnData::kString, testStrObj, &returnStatus);
	myCheckError(&returnStatus, "testString copied Correctly");
	MFnTypedAttribute meshOut;
	LSystemNode::outputMesh = meshOut.create( "outputMesh", "out", MFnData::kMesh, &returnStatus ); 
	myCheckError(&returnStatus, "ERROR creating LSystemNode output attribute\n");
	meshOut.setStorable(false);
	returnStatus = addAttribute(LSystemNode::time);
	McheckErr(returnStatus, "ERROR adding time attribute\n");

	returnStatus = addAttribute(LSystemNode::stepSize);
	McheckErr(returnStatus, "ERROR adding stepSize attribute\n");

	returnStatus = addAttribute(LSystemNode::angle);
	McheckErr(returnStatus, "ERROR adding angle attribute\n"); 
	returnStatus = addAttribute(LSystemNode::maxIterations);
	McheckErr(returnStatus, "ERROR adding maxIterations attribute\n"); 
	returnStatus = addAttribute(LSystemNode::grammar);
	myCheckError(&returnStatus, "ERROR adding grammar\n");
	returnStatus = addAttribute(LSystemNode::outputMesh);
	McheckErr(returnStatus, "ERROR adding outputMesh attribute\n");

	returnStatus = attributeAffects(LSystemNode::grammar, LSystemNode::outputMesh);
	myCheckError(&returnStatus, "ERROR in time attributeAffects");

	returnStatus = attributeAffects(LSystemNode::stepSize,
		LSystemNode::outputMesh);
	McheckErr(returnStatus, "ERROR in stepsize attributeAffects\n");
	returnStatus = attributeAffects(LSystemNode::angle,
		LSystemNode::outputMesh);
	McheckErr(returnStatus, "ERROR in angle attributeAffects\n");
	returnStatus = attributeAffects(LSystemNode::maxIterations,
		LSystemNode::outputMesh);
	McheckErr(returnStatus, "ERROR in angle attributeAffects\n");
    	returnStatus = attributeAffects(LSystemNode::time, LSystemNode::outputMesh);
	myCheckError(&returnStatus, "ERROR in time attributeAffects");

	return MS::kSuccess;
}

MObject LSystemNode::createMesh(const MTime& time,
							  MObject& outData,
							  MStatus& stat)

{
	// Scale the cube on the frame number, wrap every 10 frames.
	//
	const int frame = (int)time.as( MTime::kFilm );
	const float cubeSize = 0.5f * (float)( frame % 10 + 1 );

	MFloatPointArray points;
	points.append( -cubeSize, -cubeSize, -cubeSize );
	points.append(  cubeSize, -cubeSize, -cubeSize );
	points.append(  cubeSize, -cubeSize,  cubeSize );
	points.append( -cubeSize, -cubeSize,  cubeSize );
	points.append( -cubeSize,  cubeSize, -cubeSize );
	points.append( -cubeSize,  cubeSize,  cubeSize );
	points.append(  cubeSize,  cubeSize,  cubeSize );
	points.append(  cubeSize,  cubeSize, -cubeSize );

	MObject newMesh;

	static const bool sTestVertexIdAndFaceId =
		(getenv("MAYA_TEST_VERTEXID_AND_FACEID") != NULL);
	if (sTestVertexIdAndFaceId)
	{
		// If the env var is set, the topology of the cube will be changed over
		// frame number (looping in every 4 frames). When the shape is assigned
		// with a hwPhongShader, the shader receives vertex ids and face ids,
		// which are generated from polygonConnects passed to MFnMesh::create
		// method in this plugin.
		//
		switch (frame % 4)
		{
		case 1:
			newMesh = createQuads(points, outData, stat);
			break;
		case 2:
			newMesh = createReverseQuads(points, outData, stat);
			break;
		case 3:
			newMesh = createTris(points, outData, stat);
			break;
		case 0:
			newMesh = createReverseTris(points, outData, stat);
			break;
		default:
			newMesh = createQuads(points, outData, stat);
			break;
		}
	}
	else
	{
		newMesh = createQuads(points, outData, stat);
	}

	return newMesh;
}

MObject LSystemNode::createLSystemMesh(const unsigned int  frame, MObject& outData, MStatus& stat)
{
	std::vector<LSystem::Branch> branches;
	Connections.process(frame, branches);
	const double ratio = 0.05;
	MPointArray points;
	MIntArray faceCounts;
	MIntArray faceConnects;

	for (int i = 0; i < branches.size(); ++i)
	{
		MPoint point1(branches[i].first[0], 
				  branches[i].first[1], branches[i].first[2]);
		MPoint point2(branches[i].second[0], 
				  branches[i].second[1], branches[i].second[2]);
		double radius = point2.distanceTo(point1);
			  radius *= ratio;
		CylinderMesh cylinder(point1, point2, radius);
		cylinder.appendToMesh(points, faceCounts, faceConnects);
	}
	MFnMesh meshFS;
	return meshFS.create(points.length(), faceCounts.length(), 
				points, faceCounts, 
			   faceConnects, outData, &stat);
}

MObject LSystemNode::createQuads(
	const MFloatPointArray& points,
	MObject& outData,
	MStatus& stat)
{
	// Set up an array containing the number of vertices
	// for each of the 6 cube faces (4 vertices per face)
	//
	const int numFaces = 6;
	int face_counts[numFaces] = { 4, 4, 4, 4, 4, 4 };
	MIntArray faceCounts( face_counts, numFaces );

	// Set up and array to assign vertices from points to each face 
	//
	const int numFaceConnects = 24;
	int	face_connects[ numFaceConnects ] = {0, 1, 2, 3,
											4, 5, 6, 7,
											3, 2, 6, 5,
											0, 3, 5, 4,
											0, 4, 7, 1,
											1, 7, 6, 2};
	MIntArray faceConnects( face_connects, numFaceConnects );
	
	MFnMesh	meshFS;
	return meshFS.create(points.length(), faceCounts.length(),
		points, faceCounts, faceConnects, outData, &stat);
}

MObject LSystemNode::createReverseQuads(
	const MFloatPointArray& points,
	MObject& outData,
	MStatus& stat)
{
	// Set up an array containing the number of vertices
	// for each of the 6 cube faces (4 vertices per face)
	//
	const int numFaces = 6;
	int face_counts[numFaces] = { 4, 4, 4, 4, 4, 4 };
	MIntArray faceCounts( face_counts, numFaces );

	// Set up and array to assign vertices from points to each face
	//
	const int numFaceConnects = 24;
	int	face_connects[ numFaceConnects ] = {0, 3, 2, 1,
											4, 7, 6, 5,
											3, 5, 6, 2,
											0, 4, 5, 3,
											0, 1, 7, 4,
											1, 2, 6, 7};
	MIntArray faceConnects( face_connects, numFaceConnects );

	MFnMesh	meshFS;
	return meshFS.create(points.length(), faceCounts.length(),
		points, faceCounts, faceConnects, outData, &stat);
}
MObject LSystemNode::createTris(
	const MFloatPointArray& points,
	MObject& outData,
	MStatus& stat)
{
	// Set up an array containing the number of vertices
	// for each of the 12 triangles (3 verticies per triangle)
	//
	const int numFaces = 12;
	int face_counts[numFaces] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
	MIntArray faceCounts( face_counts, numFaces );

	// Set up and array to assign vertices from points to each face
	//
	const int numFaceConnects = 36;
	int	face_connects[ numFaceConnects ] = {0, 1, 2,
											2, 3, 0,
											4, 5, 6,
											6, 7, 4,
											3, 2, 6,
											6, 5, 3,
											0, 3, 5,
											5, 4, 0,
											0, 4, 7,
											7, 1, 0,
											1, 7, 6,
											6, 2, 1};
	MIntArray faceConnects( face_connects, numFaceConnects );

	MFnMesh	meshFS;
	return meshFS.create(points.length(), faceCounts.length(),
		points, faceCounts, faceConnects, outData, &stat);
}

MObject LSystemNode::createReverseTris(
	const MFloatPointArray& points,
	MObject& outData,
	MStatus& stat)
{
	// Set up an array containing the number of vertices
	// for each of the 12 triangles (3 verticies per triangle)
	//
	const int numFaces = 12;
	int face_counts[numFaces] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
	MIntArray faceCounts( face_counts, numFaces );

	// Set up and array to assign vertices from points to each face
	//
	const int numFaceConnects = 36;
	int	face_connects[ numFaceConnects ] = {0, 2, 1,
											2, 0, 3,
											4, 6, 5,
											6, 4, 7,
											3, 6, 2,
											6, 3, 5,
											0, 5, 3,
											5, 0, 4,
											0, 7, 4,
											7, 0, 1,
											1, 6, 7,
											6, 1, 2};
	MIntArray faceConnects( face_connects, numFaceConnects );

	MFnMesh	meshFS;
	return meshFS.create(points.length(), faceCounts.length(),
		points, faceCounts, faceConnects, outData, &stat);
}

void  LSystemNode::checkErrorNode(MStatus* latest, MString msg, bool cond)
{
	if (*latest != MStatus::kSuccess) {
		currentError = *latest;
	}
	myCheckError(latest, msg, cond);
}
MStatus LSystemNode::compute(const MPlug& plug, MDataBlock& data)

{
	MStatus returnStatus;
	unsigned int frameNo; // the frame number is the time modulated by the maxIterations

	if (plug == outputMesh) {
		if (firstTime) {
			returnStatus = updateConnection(data);
			checkErrorNode(&returnStatus, "error updating all inputs");
		        returnStatus = updateStepSize(data, frameNo);
			firstTime = false;
		}
		else {
			returnStatus =  updateStepSize(data, frameNo);
		}
		/* Get time */
	//	MDataHandle timeData = data.inputValue( time, &returnStatus ); 
	//	McheckErr(returnStatus, "Error getting time data handle\n");
	//	MTime time = timeData.asTime();


		/* Get output object */ 

		MDataHandle outputHandle = data.outputValue(outputMesh, &returnStatus);
		checkErrorNode(&returnStatus, "ERROR getting polygon data handle\n");

		MFnMeshData dataCreator;
		MObject newOutputData = dataCreator.create(&returnStatus);
		checkErrorNode(&returnStatus, "ERROR creating outputData");

		//createMesh(time, newOutputData, returnStatus);
		
		createLSystemMesh(frameNo, newOutputData, returnStatus);
		checkErrorNode( &returnStatus, "ERROR creating LSystem Cylinders");

		outputHandle.set(newOutputData);
		data.setClean( plug );
	} else
		return MS::kUnknownParameter;

	return MS::kSuccess;
}
MStatus  LSystemNode::updateConnection(MDataBlock& data)
{
	MStatus stat;
	MDataHandle allData = data.inputValue(LSystemNode::grammar, &stat);
	checkErrorNode(&stat, "testString failed to be retrieved");
	MString testm = allData.asString();
	const char * charStringm = testm.asChar();
	std::string stringM(charStringm);
	Connections.loadProgramFromString(stringM);
	allData = data.inputValue(angle, &stat);
	float angle = allData.asFloat();
	checkErrorNode(&stat, "getting angle correct");
	Connections.setDefaultAngle(angle);
	allData = data.inputValue(maxIterations, &stat);
	checkErrorNode(&stat, "maxIteration error");
	int maxIter = allData.asInt();
	return stat;
}
MStatus  LSystemNode::updateStepSize(MDataBlock& data, unsigned int& frameNo)
{
	MStatus stat;
	MDataHandle allData = data.inputValue(time, &stat);
	checkErrorNode(&stat, "Time Data input error");
	MTime time = allData.asTime();
	// time starts off at 1 but we want from a fram perspective 
	// the time to be 0
	unsigned int timeCnt = (unsigned int) time.as(MTime::kFilm) - 1;
	/* Get iteration */
	MDataHandle iterHandle = data.inputValue(maxIterations, &returnStatus);
	checkErrorNode(&returnStatus, "MaxIteration not found");
	int iter = iterHandle.asInt();
	frameNo = timeCnt % iter;
	allData = data.inputValue(stepSize, &stat);
	myCheckError(&stat, "getting StepSize correct");
	float step = allData.asFloat();
	step /= (frameNo + 1);
	Connections.setDefaultStep(step);
	return stat;
} 


void myCheckError(MStatus* stat, MString msg, bool cond)
{
	if (*stat != MS::kSuccess || !cond)
	{
		MGlobal::displayInfo(msg);
	}

}
