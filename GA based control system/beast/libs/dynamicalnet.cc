/**
 * \file dynamicalnet.cc
 * Implementation of DynamicalNet
 */

#include "dynamicalnet.h"

using namespace std;

namespace BEAST {

/**
 * Constructor, allows a DynamicalNet to be configured with the following
 * features:
 * - Variable number of input channels.
 * - Variable number of output channels.
 * - Variable total number of nodes.
 * - Option of all input channels going to all nodes, in which case each node
 * receives a weighted sum of the inputs. This defaults to True.
 * - Option of all output channels coming from all nodes, in which case each
 * node contributes a weighted value to each output. This defaults to false.
 * Input nodes are enumerated starting at the first node, although they can be
 * altered to point to different nodes. Output nodes are enumerated from the
 * last node and may also be altered to point to different channels. If 
 * inputs + outputs > total, the network will be configured with the
 * difference of nodes in the middle acting as both inputs and outputs.
 * \param i The number of input nodes.
 * \param o The number of output nodes.
 * \param t The total number of nodes.
 * \param mi True if multiple inputs per node (default true).
 * \param mo True if multiple outputs per node (default false).
 * \see DynamicalNet::Init
 * \see DynamicalNet::SetInputChannel
 * \see DynamicalNet::SetOutputChannel
 */ 
DynamicalNet::DynamicalNet(int i, int o, int t, bool mi, bool mo)
{
	Init(i, o, t, mi, mo);
}

/**
 * The destructor, does nothing at all.
 */
DynamicalNet::~DynamicalNet()
{
}

/**
 * For unserialisation purposes, the initialisation method is actually
 * responsible for configuring the network, and is called by the constructor.
 * Init also calls Reset.
 * \param i The number of input nodes.
 * \param o The number of output nodes.
 * \param t The total number of nodes.
 * \param mi True if multiple inputs per node (default true).
 * \param mo True if multiple outputs per node (default false).
 * \see DynamicalNet::Reset
 */
void DynamicalNet::Init(int i, int o, int t, bool mi, bool mo)
{
	inputs = vector<float>(i);
	outputs = vector<float>(o);
	neuronStates = vector<float>(t);
	multiInputNodes = mi;
	multiOutputNodes = mo;
	neurons.clear();

	int neuronInputs = multiInputNodes ? i : 0,
		neuronOutputs = multiOutputNodes ? o : 0,
		neuronInChl = -1,
		neuronOutChl = -1;

	for (int n = 0; n < t; ++n) {
		if (!multiInputNodes) {
			neuronInChl = n < i ? n : -1;
		}
		if (!multiOutputNodes) {
			neuronOutChl = n + (o - t);
			if (neuronOutChl < 0 || neuronOutChl >= o) {
				neuronOutChl = -1;
			}
		}
		neurons.push_back(Neuron(neuronInputs, neuronOutputs, t,
								 neuronInChl, neuronOutChl, this));
	}
	Reset();
}

/**
 * Sets the output value of each neuron (i.e. its current output) to 0.
 * This is always done on initialisation of the network.
 * \see DynamicalNet::Init
 */
void DynamicalNet::Reset()
{
	fill(neuronStates.begin(), neuronStates.end(), 0.0f);
}

/**
 * Configures the network to channel inputs to different nodes. Has no effect
 * if the net is configured with multiple input nodes per channel.
 * \param neuron The number of the node to direct the input to.
 * \param channel The number of the channel to be redirected.
 * \see DynamicalNet::SetOutputChannel
 */
void DynamicalNet::SetInputChannel(int neuron, int channel)
{
	if (multiInputNodes || channel < 0 
		|| channel > static_cast<int>(inputs.size())) return;

	vector<Neuron>::iterator i = neurons.begin();
	for (; i != neurons.end(); ++i) {
		if (i->inputChannel == channel) {
			i->inputWeights.clear();
		}
	}

	neurons[neuron].inputChannel = channel;
	neurons[neuron].inputWeights = vector<float>(1);
}

/**
 * Configures the network to channel output from different nodes. Has no
 * effect if the net is configured with multiple output nodes per channel.
 * \param neuron The number of the node to be redirected.
 * \param channel The number of the channel to redirect it to.
 * \see DynamicalNet::SetInputChannel
 */
void DynamicalNet::SetOutputChannel(int neuron, int channel)
{
	if (multiInputNodes || channel < 0 
		|| channel > static_cast<int>(outputs.size())) return;

	vector<Neuron>::iterator i = neurons.begin();
	for (; i != neurons.end(); ++i) {
		if (i->outputChannel == channel) {
			i->outputWeights.clear();
		}
	}

	neurons[neuron].outputChannel = channel;
	neurons[neuron].outputWeights = vector<float>(1);
}

/**
 * Forces each Neuron in the network to randomise itself, by calling its own
 * Randomise member function.
 * \see Neuron::Randomise
 */
void DynamicalNet::Randomise()
{
	for_each(neurons.begin(), neurons.end(), mem_fun_ref(&Neuron::Randomise));
}

/**
 * Fills all input, output and internal weights with random numbers in the
 * range [-1,1]. The time constant is also randomised with a value between
 * 1 and 70.
 */
void DynamicalNet::Neuron::Randomise()
{
	generate(inputWeights.begin(), inputWeights.end(), RandomNum);
	generate(outputWeights.begin(), outputWeights.end(), RandomNum);
	generate(weights.begin(), weights.end() - 1, RandomNum);
	bias = *(weights.end() - 2);
	timeConstant = randval(69.0f) + 1.0f;
	weights.back() = static_cast<float>(log(static_cast<double>(timeConstant)));
}

/**
 * This is where it all happens, although all the DynamicalNet class really
 * has to do is:
 * - Clear the outputs.
 * - Fire every Neuron.
 * - Retrieve and store every Neuron's output.
 * \see DynamicalNet::Neuron::Fire
 */
void DynamicalNet::Fire()
{
	// Clear the output values
	fill(outputs.begin(), outputs.end(), 0.0f);

	// Call Fire on every neuron
	for_each(neurons.begin(), neurons.end(), mem_fun_ref(&Neuron::Fire));

	// Store the output value of each neuron for next time.
	transform(neurons.begin(), neurons.end(), neuronStates.begin(), mem_fun_ref(&Neuron::GetOutput));
}

/**
 * This is where everything /actually/ happens - this method calculates the
 * amount by which the activation value changes. The code of the Firing method
 * has been carefully commented, here is a summary:
 * - We start by subtracting the last round's activation.
 * - A weighted sum of the previous neuron outputs is taken.
 * - Inputs are applied, either as a weighted sum from all channels or as an
 * individual input.
 * - The total is divided by the time constant...
 * - ... and added back to the previous activation.
 * - The new activation is then biased and squashed to produce the output.
 * - The output is applied to the relevant output channels.
 * \see Sigmoid
 */
void DynamicalNet::Neuron::Fire()
{
	// Start off with the negative of last round's activation.
	float deltaActivation = -activation;
	
	// Add weighted sum of the other neurons' **output** values
	// (output value = sigmoid(activation - bias)
	deltaActivation += inner_product(parent->neuronStates.begin(), 
									 parent->neuronStates.end(),
									 weights.begin(), 0.0f);

	// Apply any input values
	// ... if there is no particular input channel,
	if (inputChannel == -1) {
		// ... but we have input weights:
		if (!inputWeights.empty()) {
			// Add a weighted sum of the current inputs and this neuron's
			// input weights.
			deltaActivation += inner_product(parent->inputs.begin(),
											 parent->inputs.end(),
											 inputWeights.begin(), 0.0f);
		}
	}
	// ... if only one input channel goes to this node:
	else {
		// Add the unweighted input value.
		deltaActivation += parent->inputs[inputChannel];
	}

	// Divide by the time constant
	deltaActivation /= timeConstant;

	// And add to the previous activation
	activation += deltaActivation;

	// Bias and squash
	output = Sigmoid(activation - bias);

	// Send output values (if this or all neurons are output neurons)
	// ... if there is no particular output channel,
	if (outputChannel == -1) {
		// ... but we have output weights:
		if (!outputWeights.empty()) {
			// Add the neuron's output to each output channel, weighted by
			// the neuron's output weights.
			vector<float>::iterator i = parent->outputs.begin(),
									j = outputWeights.begin();
			for (; i != parent->outputs.end(); ++i, ++j) {
				*i += *j * output;
			}
		}
	}
	// ... or for just one output neuron:
	else {
		parent->outputs[outputChannel] += output;
	}
}

/**
 * Returns all the weights and biases in the network, in a long list suitable
 * for processing by a GA. Note that nearly all the values are initialised in
 * the range [-1,1], except for the time constants which range between 1 and
 * 70. The time constants are therefore stored as their natural log in the
 * configuration output which makes for more sensible alteration by the GA.
 * \return An ordered vector of floats describing the weights, biases and
 * time constants.
 * \see DynamicalNet::Neuron::GetConfiguration
 * \see DynamicalNet::SetConfiguration
 */
vector<float> DynamicalNet::GetConfiguration()const
{
	vector<float> config;
	vector<Neuron>::const_iterator i = neurons.begin();
	for (; i != neurons.end(); ++i) {
		i->GetConfiguration(config);
	}

	return config;
}

/**
 * Copies all weights, bias and time constant into the provided vector.
 * \param config A reference to the vector into which the configuration
 * must be copied.
 * \see DynamicalNet::GetConfiguration
 * \see DynamicalNet::Neuron::SetConfiguration
 */
void DynamicalNet::Neuron::GetConfiguration(vector<float>& config)const
{
	copy(inputWeights.begin(), inputWeights.end(), back_inserter(config));
	copy(outputWeights.begin(), outputWeights.end(), back_inserter(config));
	copy(weights.begin(), weights.end(), back_inserter(config));
}

/**
 * Sets the configuration of the network according to a provided vector of
 * weights, biases and time constants.
 * \param config An ordered vector of floats containing weights, biases and
 * time constants.
 * \see DynamicalNet::Neuron::SetConfiguration
 * \see DynamicalNet::GetConfiguration
 */
void DynamicalNet::SetConfiguration(const vector<float>& config)
{
	vector<Neuron>::iterator i = neurons.begin();
	vector<float>::const_iterator j = config.begin();

	do {	
		j = i->SetConfiguration(j);
	} while (++i != neurons.end());
}

/**
 * Sets the Neuron's configuration according to the input, which is an
 * iterator of a vector of floats. This has been done to enable easy
 * configuration by a DynamicalNet of its neurons, without knowing how
 * many values are required for each Neuron. The Neuron may therefore
 * take what it needs and return an iterator pointing to the rest of the
 * configuration data.
 * \param config An iterator pointing to the current position in the
 * configuration data.
 * \return An iterator pointing to the next place after this Neuron's data
 * finishes.
 */
vector<float>::const_iterator
DynamicalNet::Neuron::SetConfiguration(vector<float>::const_iterator config)
{
	// First come the input weights...
	if (inputChannel == -1) {
		copy(config, config + inputWeights.size(), inputWeights.begin());
		config += inputWeights.size();
	}
	// ... or the input weight...
	else if (!inputWeights.empty()) {
		inputWeights.front() = *config;
		++config;
	}

	// Then the output weights...
	if (outputChannel == -1) {
		copy(config, config + outputWeights.size(), outputWeights.begin());
		config += outputWeights.size();
	}
	// ... or just one output weight...
	else if (!outputWeights.empty()) {
		outputWeights.front() = *config;
		++config;
	}
	
	// Then the remainder are the internal weights
	copy(config, config + weights.size(), weights.begin());

	bias = *(weights.end() - 2);
	timeConstant = static_cast<float>(exp(static_cast<double>(weights.back())));

	if (timeConstant < 1.0f) {
		timeConstant = 1.0f + 2 * (1.0f - timeConstant);
		weights.back() = static_cast<float>(log(static_cast<double>(timeConstant)));
	}

	return config + weights.size();
}

/**
 * Outputs all setup and configuration data for this network to a stream.
 * \param out An output stream.
 * \see DynamicalNet::Unserialise
 * \todo Serialise input/output channel/node config
 */
void DynamicalNet::Serialise(ostream& out)const
{
	out << "DynamicalNet\n"
		<< setprecision(36)
		<< static_cast<int>(inputs.size()) << endl
		<< static_cast<int>(outputs.size()) << endl
		<< static_cast<int>(neurons.size()) << endl
		<< (multiInputNodes ? "multi_in" : "single_in") << endl
		<< (multiOutputNodes ? "multi_out" : "single_out") << endl;

	vector<float> config = GetConfiguration();
	copy(config.begin(), config.end(), ostream_iterator<float>(out, "\n"));
}

/**
 * Takes a string produced by DynamicalNet::Serialise and turns it back into a
 * DNN
 * \param input The input string.
 * \see DynamicalNet::Serialise
 * \todo Unserialise input/output channel/node config
 */
void DynamicalNet::Unserialise(istream& in)
{
	string name;
	in >> name;
	if (name != "DynamicalNet") {
		throw SerialException(SERIAL_ERROR_WRONG_TYPE, name, 
							  "This object is type DynamicalNet");
	}

	int i, o, t;
	bool mi, mo;

	in >> i >> o >> t >> switcher("multi_in", mi) >> switcher("multi_out", mo);
	Init(i, o, t, mi, mo);

	vector<float> config(GetConfigurationLength());
	copy_from_istream(config.begin(), config.end(), in); 
	SetConfiguration(config);
}

/**
 * Prints all the data in this network in a pretty format and returns it as a
 * string.
 * \return An STL string.
 */
string DynamicalNet::ToString()const
{
	ostringstream out;
	ostream_iterator<float> outIter(out, " ");

	out << "Input values:" << endl;
	copy(inputs.begin(), inputs.end(), outIter);
	out << endl << "Output values:" << endl;
	copy(outputs.begin(), outputs.end(), outIter);
	out << endl << "Activation states:" << endl;
	copy(neuronStates.begin(), neuronStates.end(), outIter);
	out << endl << "Neurons:" << endl;
	transform(neurons.begin(), neurons.end(), 
			  ostream_iterator<string>(out, "\n"),
			  mem_fun_ref(&Neuron::ToString));
	out << endl;

	return out.str();
}

/**
 * Prints all the data in this Neuron in a pretty format and returns it as a
 * string.
 * \return An STL string.
 */ 
string DynamicalNet::Neuron::ToString()const
{
	ostringstream out;
	ostream_iterator<float> outIter(out, " ");
	
	if (!inputWeights.empty()) {
		out << "Input weight(s):" << endl;
		copy(inputWeights.begin(), inputWeights.end(), outIter);
		out << endl;
	}
	out << "Hidden layer weight(s):" << endl;
	copy(weights.begin(), weights.end() - 2, outIter);
	out << endl << "Bias: " << bias << " Time constant: " << timeConstant
		<< endl;
	if (!outputWeights.empty()) {
		out	<< "Output weight(s):" << endl;
		copy(outputWeights.begin(), outputWeights.end(), outIter);
		out << endl;
	}
	
	return out.str();
}

/**
 * Looks at the setup data for the network and calculates the expected length
 * of the configuration data produced by GetConfiguration and expected by
 * SetConfiguration (and Unserialise)
 * \return The number of values in this network's configuration.
 * \see DynamicalNet::GetConfiguration
 * \see DynamicalNet::SetConfiguration
 */
int DynamicalNet::GetConfigurationLength()const
{
	int numNeurons = static_cast<int>(neurons.size()),
		numInputs  = static_cast<int>(inputs.size()),
		numOutputs = static_cast<int>(outputs.size());

	return numNeurons * numNeurons
		+ (multiInputNodes ? numNeurons * numInputs : 0)
		+ (multiOutputNodes ? numNeurons * numOutputs : 0);
}

/**
 * An output operator for DynamicalNet
 * \param out A reference to an output stream.
 * \param dnn A reference to a DynamicalNet.
 * \return A reference to an output stream.
 */
ostream& operator<<(ostream& out, const DynamicalNet& dnn)
{
	dnn.Serialise(out);
	return out;
}

/**
 * An input operator for DynamicalNet
 * \param in A reference to an input stream.
 * \param dnn A reference to a DynamicalNet.
 * \return A reference to an input stream.
 */
istream& operator>>(istream& in, DynamicalNet& dnn)
{
	dnn.Unserialise(in);
	return in;
}

} // namespace BEAST
