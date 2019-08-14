#include "LSystemCmd.h"
#include <maya/MPoint.h>
#include <maya/MGlobal.h>
#include <maya/MPointArray.h>
#include <maya/MfnNurbsCurve.h>
#include <maya/MargList.h>
#include <list>
#include "LSystemNode.h"
#include <maya/MDagModifier.h>
#include <maya/MDGModifier.h>
#include <maya/MPlug.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnStringData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MSelectionList.h>
LSystemCmd::LSystemCmd() : MPxCommand(), step {1, MDistance::Unit::kCentimeters}, 
	angle{ 60, MAngle::Unit::kDegrees },
	grammar{" F + F"}, iter {1} 
{
}

LSystemCmd::~LSystemCmd() 
{
}

MStatus LSystemCmd::doIt(const MArgList& args)
{
	// message in Maya output window
	cout << "Implement Me!" << endl;
	std::cout.flush();
	MPoint circleCenter{ 0, 0, 0 };
	MPoint circleNormal{ 0, 1.0, 0 };
	float   radius { 1.0f};
	parseArguments(args);
	MGlobal::executeCommand(MString("circle -c ") + circleCenter.x + " " +
		circleCenter.y + " " + circleCenter.z + " " + " -nr " + 
		circleNormal.x + " " + circleNormal.y + " " + circleNormal.z + " " +
		" -r " + radius + "-d 3 -ut 0 -tol 0.01 -s 8 -ch 1 -n nurbsCircle1", true);
	const unsigned  deg = 1;
	const unsigned ncvs = 2;
    	const unsigned spans = ncvs - deg;
	const unsigned nknots = spans + 2 * deg - 1; // number of knots
	MPointArray controlVertices;
	MDoubleArray knotSequences;
	for (unsigned i = 0; i < nknots; ++i)
	{ 
		knotSequences.append(static_cast<double>(i));
	}
	controlVertices.append(MPoint(0., 0., 0.));
	controlVertices.append(MPoint(1., 1., 1.));
	MFnNurbsCurve  curveFn;
	MStatus stat;
	MObject curve = curveFn.create(controlVertices, knotSequences, deg,
		MFnNurbsCurve::kOpen, false, false, MObject::kNullObj, &stat);
	MString name = curveFn.name();
	MGlobal::displayInfo(name);
	if (stat != MS::kSuccess)
	{
		MGlobal::displayInfo("curve creation failed");
	}
	// message in scriptor editor
	MGlobal::executeCommand(MString("extrude - ch true - rn false - po 1 - et 2 "
		"- ucp 1 - fpt 1 - upn 1 - rotation 0" 
		"- scale 1 - rsp 1 ") + "nurbsCircle1 " + name, true);
	MSelectionList lst;
	stat = lst.add("LSystemNode1");
	myCheckError(&stat, "LSystemNode1 not found");
	int i = lst.length(&stat);
	myCheckError(&stat, "Length not 1", i == 1);
	MObject LSNode;
	stat = lst.getDependNode(0, LSNode);
	myCheckError(&stat, "getting first node from list failed");
	stat = updateLSystemNodeAttributes(LSNode);
	myCheckError(&stat, "failure in updating attributes");
	//CreateNode();
	MGlobal::displayInfo("plug in over");


    return stat;

}
// does add a dynamic attribute
MStatus   LSystemCmd::addGrammarAttribute(MObject LSnode) const
{
	MFnTypedAttribute typedAttr;
	MFnStringData  sdata;
	MStatus stat;
	MDGModifier mdg;
	MObject stringObj = sdata.create(grammar, &stat);
	myCheckError(&stat, "String Object not created properly");
	MObject gramAttr1 = typedAttr.create("grammar", "g", MFnData::kString, stringObj, &stat);
	//MObject gramAttr1 = typedAttr.create("grammar", "g", MFnData::kString, MObject::kNullObj, &stat);
	myCheckError(&stat, "grammar attribute not created properly");
	stat = mdg.addAttribute(LSnode, gramAttr1 );
	myCheckError(&stat, "typed Attribute not attached correctly");
	mdg.doIt();
	MPlug plug(LSnode, gramAttr1);
	MString test = "";
	stat = plug.getValue(test);
	myCheckError(&stat, "stored value not correct", test == grammar);
	// this is another way to add grammar to the attribute
//	stat = plug.setValue(grammar);
//	myCheckError(&stat, "Grammar plug not set properly");
//	MString test = "";
//	plug = MPlug(LSnode, gramAttr1);
//	stat = plug.getValue(test);
//	myCheckError(&stat, "plugin value not set correctly", test == grammar);
	//mdg.connect(LSnode, outputMesh, shape, inMesh);
	return stat;
}
// create a new "grammar" plug and connect it to the grammar plug of LSnode 
MStatus   LSystemCmd::connectGrammarAttribute(MObject LSnode, MPlug gplug) const
{
	MFnTypedAttribute typedAttr;
	MFnStringData  sdata;
	MStatus stat;
	MDGModifier mdg;
	MObject stringObj = sdata.create(grammar, &stat);
	myCheckError(&stat, "String Object not created properly");
	MObject gramAttr1 = typedAttr.create("tempG", "tg", MFnData::kString, stringObj, &stat);
	myCheckError(&stat, "grammar attribute not created properly");
	MObject   plugObject = mdg.createNode(MString("MPxNode"), &stat);
	myCheckError(&stat, "Node failed to create");
	MPlug attributePlug(plugObject, gramAttr1);
	MString testvalue;
	stat = attributePlug.getValue(testvalue);
	myCheckError(&stat, "no value from new plug", testvalue == grammar);
//	stat = attributePlug.setAttribute(gramAttr1);
	bool isNet = attributePlug.isConnected();
	isNet = gplug.isConnected();
	myCheckError(&stat, "Plug does not hold grammar attribute");
	MFnDependencyNode MFnLSnode(LSnode, &stat);
	stat = mdg.connect(attributePlug, gplug);
	myCheckError(&stat, "grammar plugs not connected correctly");
	stat = mdg.doIt();
	myCheckError(&stat, "connection not successful");
	return stat;
}
MStatus LSystemCmd::CreateNode() const
{
	MDagModifier mdag;
	MDGModifier mdg;
	MStatus stat;
	MObject transform = mdag.createNode(MString("transform"), MObject::kNullObj, &stat);
	myCheckError(&stat, "transform failed");
	MString msg;
	MObject shape = mdag.createNode(MString("mesh"), transform, &stat);
	myCheckError(&stat, "shape failed");
	stat = mdag.doIt();
	myCheckError(&stat, "MDag Error");
    	MObject LSnode = mdg.createNode(0x7ffff, &stat);
	myCheckError(&stat, "MDG failed");
 	stat = updateLSystemNodeAttributes(LSnode);	
	MFnDependencyNode mfnDep(LSnode, &stat);
//	stat = connectGrammarAttribute(LSnode, MPlugLSNode);
	MPlug MPlugLSNode = mfnDep.findPlug("outputMesh", true, &stat);
	myCheckError(&stat, "outputMesh plug not found");
	stat = connectNode(LSnode, shape);
	return stat;
}
MStatus LSystemCmd::updateLSystemNodeAttributes( MObject LSnode) const
{
	MStatus stat;
	MFnDependencyNode mfnDep(LSnode, &stat);
	myCheckError(&stat, "dependency FnSet failed");
	MPlug MPlugLSNode = mfnDep.findPlug("StepSize", true, &stat);
	myCheckError(&stat, "Step size failed");
	float value;
    	stat =  MPlugLSNode.getValue(value);
	myCheckError(&stat, "Step size not correct");
	stat = MPlugLSNode.setValue(step.asCentimeters());
    	stat =  MPlugLSNode.getValue(value);
	myCheckError(&stat, "value disagreement", value == step.asCentimeters());
	MPlugLSNode = mfnDep.findPlug("angle", true, &stat);
	stat = MPlugLSNode.setValue(angle.asDegrees());
	stat = MPlugLSNode.getValue(value);
	myCheckError(&stat, "Degree Error", value == angle.asDegrees());
	MPlugLSNode = mfnDep.findPlug("grammar", true, &stat);
	MString testS;
	stat = MPlugLSNode.getValue(testS);
	myCheckError(&stat, "GrammarPlug", testS == "F+F+");
	stat = MPlugLSNode.setValue(grammar);
	myCheckError(&stat, "GrammarPlug Error");
	stat = MPlugLSNode.getValue(testS);
	myCheckError(&stat, "GrammarPlug Error", testS == grammar);
	MPlugLSNode = mfnDep.findPlug("iterations", true, &stat);
	myCheckError(&stat, "max iterations failed failed");
	stat = MPlugLSNode.setValue(iter);
	int setVal;
	stat = MPlugLSNode.getValue(setVal);
	myCheckError(&stat, "iteration Error", setVal == iter);
	return stat;
}

MStatus LSystemCmd::connectNode(MObject LSnode, MObject meshNode) const
{
	// connect the mes
	MStatus stat;
	MFnDependencyNode MFnLSnode(LSnode, &stat);
	myCheckError(&stat, "LSNode not attached correctly");
	MFnDependencyNode MFnMesh(meshNode, &stat);
	myCheckError(&stat, "MFnMesh not attached correctly");
	MPlug LSplug = MFnLSnode.findPlug("outputMesh", &stat);
	myCheckError(&stat, "outputMesh not found");
	MPlug meshPlug = MFnMesh.findPlug("inMesh", &stat);
	myCheckError(&stat, "inmesh not found");
	MDGModifier mdg;
	stat = mdg.connect(LSplug, meshPlug);
	myCheckError(&stat, " connect error mdg");
	stat = mdg.doIt();
	myCheckError(&stat, "mdg doit error");
	//connect the time;
	LSplug = MFnLSnode.findPlug("time", &stat);
	myCheckError(&stat, "time not found");
	MSelectionList lst;
	stat = lst.add("time1");
	myCheckError(&stat, "time1 not found");
	int i = lst.length(&stat);
	myCheckError(&stat, "Length not 1", i == 1);
	MObject timeNode;
	stat = lst.getDependNode(0, timeNode);
	myCheckError(&stat, "getting first node from list failed");
	
	MFnDependencyNode MFnTime(timeNode, &stat);
	myCheckError(&stat, "MFn failed to initialize with the time node");
	MPlug timePlug = MFnTime.findPlug("outTime", &stat);
	myCheckError(&stat, "outTime  not found");
	bool isConnected = LSplug.isConnected();
	stat = mdg.connect(timePlug, LSplug);
	myCheckError(&stat, "Time failed to connect ");
	stat = mdg.doIt();
	myCheckError(&stat, "time connection failed to be implemented");
	isConnected = LSplug.isConnected();
	return stat;
}
MStatus LSystemCmd::parseArguments(const MArgList& args)
{
		MStatus status;
		int iterT;
		double x;
		MString temp;
		// first arg is the name
		for (int i = 0; i < args.length(); i++)
		{
			if (MString("-grammar") == args.asString(i, &status) && MS::kSuccess == status) {
				temp = args.asString(++i, &status);
				if (MS::kSuccess == status) {
					grammar = temp;
				}
			}
			if (MString("-iter") == args.asString(i, &status) && MS::kSuccess == status) {
				iterT = args.asInt(++i, &status);
				if (MS::kSuccess == status)
				{
					iter = iterT;
				}
			}
			if (MString("-step") == args.asString(i, &status) && MS::kSuccess == status) {
				x = args.asDouble(++i, &status);
				if (MS::kSuccess == status)
				{
					step = MDistance(x, MDistance::Unit::kCentimeters);
				}
			}
			if (MString("-angle") == args.asString(i, &status) && MS::kSuccess == status) {
				x = args.asDouble(++i, &status);
				if (MS::kSuccess == status)
				{
					angle = MAngle(x, MAngle::Unit::kDegrees);
				}
			}
		}
		MGlobal::displayInfo("Hello World\n");
		MString display = "******  Dialogue Box ***********************\\n";
		display += "Grammar: " + grammar + "\\niter: " + iter +
		      "\\nStep (cm): " + step.asCentimeters() + "; " + "\\nAngle (degrees): " 
			       + angle.asDegrees();
		MGlobal::displayInfo(display);
		MString commandStr("confirmDialog -title \"********User Input********\" -message  ");
		commandStr += "\"" + display + "\"";
		MGlobal::executeCommand(commandStr, true);
		
		return MS::kSuccess;
}
