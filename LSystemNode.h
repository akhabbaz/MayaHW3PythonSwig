// This file is derived from the bottom file

//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+


////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
// This will contain 
// 
// This plug-in produces the dependency graph node "LSystemNode".
// 
// It demonstrates how to take time as an input, and create polygonal geometry for output.
// The compute method of the node constructs a polygonal cube whose size depends on the current frame number.
// The resulting mesh is passed to an internal Maya node, which displays it and allows it to be positioned.
// 
// To use this node, execute the MEL command "LSystemNode.mel" that contains the following commands:
//
// createNode transform -n LSystemNode;
// createNode mesh -n LSystemNodeShape1 -p LSystemNode1;
// createNode LSystemNode -n LSystemNodeNode1;
// connectAttr time1.outTime LSystemNodeNode1.time;
// connectAttr LSystemNodeNode1.outputMesh LSystemNodeShape1.inMesh;
// 
// This creates a mesh node under a transform node that is hooked into the world for display.
// It then creates an LSystemNode node and connects its input to the time node, and its output to the mesh node.
// Now, a cube appears on the screen.
// 
// If the play button on the time slider is pressed, the displayed cube will grow and shrink as the frame number changes.
// 
////////////////////////////////////////////////////////////////////////
#pragma once  
#include <maya/MPxNode.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MFnMeshData.h>
#include <maya/MIOStream.h>
#include "LSystem.h"



class LSystemNode : public MPxNode
{
public:
	LSystemNode();
	~LSystemNode() override;
	MStatus compute(const MPlug& plug, MDataBlock& data) override;
	static  void*	creator();
	static  MStatus initialize();
	static MObject	time;
	static MObject	stepSize;
	static MObject   grammar;
	static MObject  angle;
	static MObject  maxIterations;
	static MObject	outputMesh;
	static MTypeId	id;
	MStatus currentError;
	LSystem  Connections;
	bool     firstTime;
	MStatus  updateConnection(MDataBlock& data);
	MStatus  updateStepSize(MDataBlock& data, unsigned int& frameNo);
	// keeps runs myCheckError and stores latest error
	void  checkErrorNode(MStatus* latest, MString msg = "", bool cond = true);

protected:
	MObject createMesh(const MTime& time, MObject& outData, MStatus& stat);

	// Helpers
	MObject createQuads(const MFloatPointArray& points, MObject& outData, MStatus& stat);
	MObject createReverseQuads(const MFloatPointArray& points, MObject& outData, MStatus& stat);
	MObject createTris(const MFloatPointArray& points, MObject& outData, MStatus& stat);
	MObject createReverseTris(const MFloatPointArray& points, MObject& outData, MStatus& stat);
	MObject createLSystemMesh(unsigned int frameNo, MObject& outData, MStatus& stat);
};

// will display message if status is not ok or if cond is not true
void myCheckError(MStatus* stat, MString msg = "", bool cond = true);


