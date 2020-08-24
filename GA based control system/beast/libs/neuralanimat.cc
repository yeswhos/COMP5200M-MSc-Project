#include "neuralanimat.h"

namespace BEAST {

/**
 * Destructor for FFNAnimat, if the FeedForwardNet has been initialised,
 * it is deleted here.
 */
FFNAnimat::~FFNAnimat()
{
	if (myBrain != NULL && ownBrain) delete myBrain;
}

/**
 * This method is responsible for initialising the FFNAnimat's neural network.
 * It should usually be called in the constructor of a derived class, after
 * the sensors have been set up. Also randomises the neural network for use in
 * evolutionary simulations.
 * \param hidden The number of hidden nodes, defaults to be the same as the
 * number of inputs.
 * \param inputs The number of inputs, defaults to be the same as the number
 * of sensors on the Animat.
 * \param outputs The number of outputs, defaults to be the same as the number
 * of controls on the Animat.
 */
void FFNAnimat::InitFFN(int hidden, int inputs, int outputs)
{
	if (hidden == -1)	hidden = static_cast<int>(GetSensors().size());
	if (inputs == -1)	inputs = static_cast<int>(GetSensors().size());
	if (outputs == -1)	outputs = static_cast<int>(GetControls().size());

	myBrain = new FeedForwardNet(inputs, outputs, hidden);
	myBrain->Randomise();
}

/**
 * The FFNAnimat's neural net is linked to its sensors and controls here. All
 * sensor inputs are fed to the neural network and all control outputs are
 * taken from the ANN's output values.
 * \warning It is assumed that there are at least as many input nodes as
 * sensors and at least as many output nodes as controls. If your Animat is
 * not set up in this way your needs are likely greater than can be provided
 * for by FFNAnimat.
 */
void FFNAnimat::Control()
{
	int n = 0;
	for (SensorIter i = GetSensors().begin(); i != GetSensors().end(); ++i, ++n) {
		myBrain->SetInput(n, static_cast<float>(i->second->GetOutput()));
	}

	myBrain->Fire();

	n = 0;
	for (ControlIter i = GetControls().begin(); i != GetControls().end(); ++i, ++n) {
		i->second = myBrain->GetOutput(n);
	}

	Animat::Control();
}

void FFNAnimat::SetBrain(FeedForwardNet& ffn)
{
	if (ownBrain && myBrain != &ffn) delete myBrain;
	
	myBrain = &ffn;
}

/**
 * Outputs the FFNAnimat's data to a stream.
 * \param out A reference to an output stream.
 */
void FFNAnimat::Serialise(std::ostream& out)const
{
	out << "FFNAnimat\n";
	Animat::Serialise(out);
	out	<< *myBrain;
}

/**
 * Inputs the FFNAnimat's data from a stream.
 * \param in A reference to an input stream.
 */
void FFNAnimat::Unserialise(std::istream& in)
{
	std::string name;
	in >> name;
	if (name != "FFNAnimat") {
		throw SerialException(SERIAL_ERROR_WRONG_TYPE, name, 
							  "This object is type FFNAnimat");
	}

	Animat::Unserialise(in);

	if (myBrain != NULL) delete myBrain;
	myBrain = new FeedForwardNet(0, 0);
	in >> *myBrain;
}

/**
 * Destructor for DNNAnimat, if the DynamicalNet has been initialised,
 * it is deleted here.
 */
DNNAnimat::~DNNAnimat()
{
	if (myBrain != NULL) delete myBrain;
}

/**
 * This method is responsible for initialising the DNNAnimat's neural network.
 * It should usually be called in the constructor of a derived class, after
 * the sensors have been set up. Also randomises the neural network for use in
 * evolutionary simulations.
 * \param hidden The number of nodes, defaults to be the same as the number of
 * sensors on the Animat.
 * \param inputs The number of inputs, defaults to be the same as the number
 * of sensors on the Animat.
 * \param outputs The number of outputs, defaults to be the same as the number
 * of controls on the Animat.
 * \param multiInput Set to true if all inputs go to all nodes, false if not,
 * defaults to true.
 * \param multiOutput Set to true if all outputs come from all nodes, false if
 * not, defaults to false.
 */
void DNNAnimat::InitDNN(int total, int inputs, int outputs,
						bool multiInput, bool multiOutput)
{
	if (total == -1)	total = static_cast<int>(GetSensors().size());
	if (inputs == -1)	inputs = static_cast<int>(GetSensors().size());
	if (outputs == -1)	outputs = static_cast<int>(GetControls().size());

	myBrain = new DynamicalNet(inputs, outputs, total, multiInput, multiOutput);
	myBrain->Randomise();
}

/**
 * The DNNAnimat's neural net is linked to its sensors and controls here. All
 * sensor inputs are fed to the neural network and all control outputs are
 * taken from the ANN's output values.
 * \warning It is assumed that there are at least as many input channels as
 * sensors and at least as many output channels as controls. If your Animat is
 * not set up in this way your needs are likely greater than can be provided
 * for by DNNAnimat.
 */
void DNNAnimat::Control()
{
	int n = 0;
	for (SensorIter i = GetSensors().begin(); i != GetSensors().end(); ++i, ++n) {
		myBrain->SetInput(n, static_cast<float>(i->second->GetOutput()));
	}

	myBrain->Fire();

	n = 0;
	for (ControlIter i = GetControls().begin(); i != GetControls().end(); ++i, ++n) {
		i->second = myBrain->GetOutput(n);
	}

	Animat::Control();
}

void DNNAnimat::SetBrain(DynamicalNet& dnn)
{
	if (ownBrain && myBrain != &dnn) delete myBrain;
	
	myBrain = &dnn;
}

/**
 * Outputs the DNNAnimat's data to a stream.
 * \param out A reference to an output stream.
 */
void DNNAnimat::Serialise(std::ostream& out)const
{
	out << "DNNAnimat\n";
	Animat::Serialise(out);
	out	<< *myBrain;
}

/**
 * Inputs the DNNAnimat's data from a stream.
 * \param in A reference to an input stream.
 */
void DNNAnimat::Unserialise(std::istream& in)
{
	std::string name;
	in >> name;
	if (name != "DNNAnimat") {
		throw SerialException(SERIAL_ERROR_WRONG_TYPE, name, 
							  "This object is type DNNAnimat");
	}

	Animat::Unserialise(in);

	if (myBrain != NULL) delete myBrain;
	myBrain = new DynamicalNet(0, 0, 0);
	in >> *myBrain;
}

} // namespace BEAST

