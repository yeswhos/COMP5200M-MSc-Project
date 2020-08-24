/**
 * \file simulation.cc
 * \author David Gordon
 * Implementation of the Simulation class.
 */

#include "simulation.h"

using namespace std;

namespace BEAST {

/**
 * The current run is set to 0, the complete flag becomes false and each
 * SimObject is given a pointer to theWorld, then the run begins.
 */
void Simulation::Init()
{
	run = 0;
	complete = false;

	map<string, SimObject*>::iterator i = contents.begin();
	for (; i != contents.end(); ++i) {
		i->second->SetWorld(&theWorld);
	}

	BeginRun();
}

/**
 * Updates the World once and checks to see if the time limit has passed. If
 * the time limit has passed, EndAssessment is called.
 */
bool Simulation::Update()
{
	theWorld.Update();
	if (++timeStep == timeSteps) EndAssessment();
	return !complete;
}

/**
 * This method is called when the GUI needs to break the current assessment
 * and start again from the beginning of that assessment.
 */
void Simulation::ResetAssessment()
{
	theWorld.CleanUp();
	BeginAssessment();
}

/**
 * This method is called when the GUI needs to break the current generation
 * and start again from the beginning of that generation.
 */
void Simulation::ResetGeneration()
{
	theWorld.CleanUp();
	BeginGeneration();
}

/**
 * This method is called when the GUI needs to break the current run
 * and start again from the beginning of that run.
 */
void Simulation::ResetRun()
{
	theWorld.CleanUp();
	BeginRun();
}

/**
 * This method is called at the beginning of every assessment. If you want to
 * override this method, ensure that Simulation::BeginAssessment is called
 * at the end of your overridden version.
 */
void Simulation::BeginAssessment()
{
	timeStep = 0;

	for_each(contents.begin(), contents.end(), 
			 call_on_mem(&pair<const string, SimObject*>::second,
						 mem_fun(&SimObject::BeginAssessment)));

	theWorld.Init();
}

/**
 * This method is called at the end of every assessment. If you want to
 * override this method, ensure that Simulation::EndAssessment is called
 * at the end of your overridden version.
 */
void Simulation::EndAssessment()
{
	theWorld.CleanUp();

	// Call NextAssessment on every simulation object
	for_each(contents.begin(), contents.end(), 
			 call_on_mem(&pair<const string, SimObject*>::second,
						 mem_fun(&SimObject::EndAssessment)));

	if (logStream != NULL) {
		*logStream << ToString(SIM_PRINT_ASSESSMENT);
	}

	if (++assessment == assessments) {
		EndGeneration();
	}
	else {
		BeginAssessment();
	}
}

/**
 * This method is called at the beginning of every generation. If you want to
 * override this method, ensure that Simulation::BeginGeneration is called
 * at the end of your overridden version.
 */
void Simulation::BeginGeneration()
{	
	assessment = 0;

	// Call BeginGeneration on every simulation object
	for_each(contents.begin(), contents.end(), 
			 call_on_mem(&pair<const string, SimObject*>::second,
						 mem_fun(&SimObject::BeginGeneration)));

	BeginAssessment();
}

/**
 * This method is called at the end of every generation. If you want to
 * override this method, ensure that Simulation::EndGeneration is called
 * at the end of your overridden version.
 */
void Simulation::EndGeneration()
{
	// Call NextGeneration on every simulation object
	for_each(contents.begin(), contents.end(), 
			 call_on_mem(&pair<const string, SimObject*>::second,
						 mem_fun(&SimObject::EndGeneration)));

	if (logStream != NULL) {
		*logStream << ToString(SIM_PRINT_GENERATION);
	}

	if (++generation == generations) {
		EndRun();
	}
	else {
		BeginGeneration();
	}
}

/**
 * This method is called at the beginning of every run. If you want to
 * override this method, ensure that Simulation::BeginRun is called at the
 * end of your overridden version. 
 */
void Simulation::BeginRun()
{
	generation = 0;

	// Call BeginRun on every simulation object
	for_each(contents.begin(), contents.end(), 
			 call_on_mem(&pair<const string, SimObject*>::second,
						 mem_fun(&SimObject::BeginRun)));

	BeginGeneration();
}

/**
 * This method is called at the end of every run and is responsible for
 * either stopping the simulation if the maximum number of runs has been
 * reached. If you want to override this method, ensure that
 * Simulation::EndRun is called at the end of your overridden version.
 */
void Simulation::EndRun()
{
	// Call EndRun on every simulation object
	for_each(contents.begin(), contents.end(), 
			 call_on_mem(&pair<const string, SimObject*>::second,
						 mem_fun(&SimObject::EndRun)));

	if (logStream != NULL) {
		*logStream << ToString(SIM_PRINT_RUN);
	}

	if (++run == runs) {
		complete = true;
	}
	else {
		BeginRun();
	}
}

/**
 * Reports a few details about the current state of the simulation.
 * \return The output string.
 */
std::string Simulation::ToString(SimPrintStyleType s)const
{
	ostringstream out;

	switch (s) {
	case SIM_PRINT_STATUS:
		if (complete) {
			return "Simulation complete";
		}
		else {		
			if (runs != 1) out << "Run: " << (run + 1) << '/' << runs << ", ";
			out << "Generation: " << (generation + 1);
			if (generations != 0) out << '/' << generations;
			out << ", ";
			if (assessments != 1) out << "Assessment: " << (assessment + 1) << '/' 
									  << assessments << ", ";
			out << "Time step: " << (timeStep + 1) << '/' << timeSteps;
		}
		break;
	case SIM_PRINT_ASSESSMENT:
		break;
	case SIM_PRINT_GENERATION:
//		transform(contents.begin(), contents.end(),
//				  ostream_iterator<string>(out),
//				  call_on_mem(&pair<const string, SimObject*>::second,
//							  mem_fun(&SimObject::ToString)));

		for (map<string, SimObject*>::const_iterator i = contents.begin();
			 i != contents.end(); ++i) {
			out << i->second->ToString();
		}

		break;
	case SIM_PRINT_RUN: break;
	case SIM_PRINT_COMPLETE: break;
	default: break;
	}	

	return out.str();
}

/**
 * Uses the SimObject's Serialise method (selected through polymorphism)
 * to stream the object to the specified file.
 * \return True if successful.
 */
bool SimObject::Save(const char* fileName)const
{
	ofstream file(fileName, ios::out | ios::trunc);

	if (file.is_open() && file.good()) {
		Serialise(file);
		return true;
	}
	else {
		throw SerialException(SERIAL_ERROR_BAD_FILE, fileName,
							  "Error occurred while saving file.");
	}
}

/**
 * Uses the SimObject's Unserialise method (selected through
 * polymorphism) to reinstate the object from the specified file.
 * \return True if successful.
 */
bool SimObject::Load(const char* fileName)
{
	ifstream file(fileName);

	if (file.is_open() && file.good()) {
		Unserialise(file);
		return true;
	}
	else {
		throw SerialException(SERIAL_ERROR_BAD_FILE, fileName,
							  "Error occurred while loading file.");
	}
}

/**
 * An output operator for all classes derived from SimObject.
 * \param out An output stream.
 * \param obj An object derived from SimObject.
 * \return A reference to output stream.
 */
ostream& operator<<(ostream& out, const SimObject& obj)
{
	obj.Serialise(out);
	return out;
}

/**
 * An input operator for all classes derived from SimObject.
 * \param in An input stream.
 * \param obj An object derived from SimObject.
 * \return A reference to input stream.
 */
istream& operator>>(istream& in, SimObject& obj)
{
	obj.Unserialise(in);
	return in;
}

} // namespace BEAST
