#include "feedforwardnet.h"

using namespace std;

namespace BEAST {

/**
 * The constructor for FeedForwardNet, which configures an empty network with
 * the specified dimensions and features.
 * \param in The number of inputs.
 * \param out The number of outputs.
 * \param hig The size of the hidden layer. If hid is 0 then the FFN acts as
 * a perceptron.
 * \param sig Whether or not the net will use a sigmoid activation function,
 * defaults to true.
 * \param bias Whether of not each node has a bias value, defaults to true.
 */
FeedForwardNet::FeedForwardNet(int in, int out, int hid, bool sig, bool bias)
{
	Init(in, out, hid, sig, bias);
}

/**
 * Initialises the FeedForwardNet with the specified dimensions and features.
 * After this method is called, all weights in the net will be set to 0.
 * \param in The number of inputs.
 * \param out The number of outputs.
 * \param hig The size of the hidden layer. If hid is 0 then the FFN acts as
 * a perceptron.
 * \param sig Whether or not the net will use a sigmoid activation function,
 * defaults to true.
 * \param bias Whether of not each node has a bias value, defaults to true.
 */
void FeedForwardNet::Init(int in, int out, int hid, bool sig, bool bias)
{
	inputs = in;
	outputs = out;
	hidden = hid;
	sigmoid = sig;
	biasNode = bias;
	inputValues = vector<float>(in);
	outputValues = vector<float>(out);
	hiddenLayer.clear();
	outputLayer.clear();

	// For each hidden layer neuron, we instantiate a Neuron object.
	// The Neuron is initialised with the number of weights it needs,
	// which in this case is one per input. If biasNode has been set
	// to true, an extra weight is added. The use of this is explained
	// in the fire method.
	for (int i=0; i<hidden; ++i) {
		hiddenLayer.push_back(Neuron(inputs + (biasNode ? 1 : 0) ));
	}

	// If the hidden layer size is set to 0, normally that would break the
	// network but here I'm taking it to mean the net is a perceptron (one
	// layer of inputs, one layer of outputs, no hidden layer) and so the
	// output layer neurons have <inputs>, rather than <hidden>, inputs.
	if (hid == 0) hid = inputs;
	// Add one neuron per output value to the output layer, each with
	// one weight per hidden neuron (since this is the output layer,
	// inputs are coming from the hidden layer) and again an extra
	// weight if biasNode is set to true.
	for (int i=0; i<outputs; ++i) {
		outputLayer.push_back(Neuron(hid + (biasNode ? 1 : 0) ));
	}
}

/**
 * The destructor for FeedForwardNet, doesn't do very much at all.
 */
FeedForwardNet::~FeedForwardNet()
{
}

/**
 * Initialises every weight and bias in the net with a random number in the
 * range [-1,1].
 */
void FeedForwardNet::Randomise()
{
	vector<Neuron>::iterator i;

	for (i = hiddenLayer.begin(); i != hiddenLayer.end(); ++i) {
		generate(i->weights.begin(), i->weights.end(), RandomNum);
	}

	for (i = outputLayer.begin(); i != outputLayer.end(); ++i) {
		generate(i->weights.begin(), i->weights.end(), RandomNum);
	}
}

/**
 * Returns all the weights and biases in the network as a vector of floats,
 * ideal for representing the network in an evolutionary algorithm.
 * No information about the dimensions of the network is returned by this
 * method - to return complete configuration data use Serialise.
 * \return A vector containing the weights and biases of the network.
 * \see FeedForwardNet::SetConfiguration
 * \see FeedForwardNet::Serialise
 */
vector<float> FeedForwardNet::GetConfiguration()const
{
	// The configuration data contains no information about the
	// network's dimensions, bias nodes, or activation function.

	vector<float> config;
	vector<Neuron>::const_iterator i;

	// Using the STL copy template algorithm instead of a for loop. Wow!
	for (i = hiddenLayer.begin(); i != hiddenLayer.end(); ++i) {
		copy(i->weights.begin(), i->weights.end(), back_inserter(config));
	}

	for (i = outputLayer.begin(); i != outputLayer.end(); ++i) {
		copy(i->weights.begin(), i->weights.end(), back_inserter(config));
	}

	return config;
}

/**
 * Takes the output of GetConfiguration and configures the weights and biases
 * of the network accordingly. Note that since no data about the dimensions of
 * the network is stored in the output of GetConfiguration, this method
 * assumes that the network has already been set up with dimensions matching
 * those of the input configuration.
 * \config A vector containing weights and biases.
 * \see FeedForwardNet::GetConfiguration
 * \see FeedForwardNet::Unserialise
 */
void FeedForwardNet::SetConfiguration(const vector<float>& config)
{
	// The configuration data contains no information about the
	// network's dimensions, bias nodes, or activation function.

	// First ensure the incoming vector is the right size
	unsigned int expectedSize = static_cast<unsigned int>(GetConfigurationLength());

	if (config.size() != expectedSize) return; // Error to go here

	vector<Neuron>::iterator i;
	vector<float>::const_iterator configIter(config.begin());

	// Now we move through config, assigning series of size inputs + biasnode
	// to the weights of each Neuron.

	for (i = hiddenLayer.begin(); i != hiddenLayer.end(); ++i) {
		i->weights = vector<float>(configIter, configIter+inputs+(biasNode?1:0));
		configIter += inputs + (biasNode ? 1 : 0);
	}

	// Same again for output layer
	for (i = outputLayer.begin(); i != outputLayer.end(); ++i) {
		i->weights = vector<float>(configIter, configIter+hidden+(biasNode?1:0));
		configIter += hidden + (biasNode ? 1 : 0);
	}

	// One more error check:
	if (configIter != config.end()) true; // Another error

	return;
}

/**
 * Returns the sum of each value of the input vector, multiplied by the
 * Neuron's respective weight values.
 * \param input A vector of floats.
 */
float FeedForwardNet::Neuron::WeightedSum(vector<float>& input)const
{
	if (input.size() > weights.size()) {
		std::cerr << "Error: too many inputs!" << std::endl;
	}

	// All we're doing here is moving through the input values (which will
	// always be either the net's inputs or the outputs of the hidden layer)
	// and multiplying them by their respective weights
	return inner_product(input.begin(), input.end(), weights.begin(), 0.0f);

	// Note that the output from this function isn't ready to be passed onto
	// the next layer yet - this weighted sum needs to be (optionally) biased
	// and put through the activation function - either sigmoid or threshold.
}

/**
 * This is the main method of the Feed Forward Network, where inputs are
 * processed to calculated the output values. The Fire method assumes that
 * inputs have previously been set using SetInput.
 * \see FeedForwardNet::SetInput
 */
void FeedForwardNet::Fire()
{
	vector<float> hiddenOutput;			// A holding space for the output
	outputValues.clear();				// of the hidden layer, to pass to
	float output;						// the output layer neurons.
	
	// If the size of the hidden layer is set to 0, this particular ANN class
	// recognises that to mean there is no hidden layer, and so the input
	// values should be processed directly by the output layer. When this
	// happens - i.e. there is only one round of neural processing - you have
	// a perceptron. Normally a zero-neuron hidden layer would just break the
	// network, but here we're forcing something else to happen.
	if (hidden == 0) hiddenOutput = inputValues;

	vector<Neuron>::const_iterator currentNeuron(hiddenLayer.begin());
	
	for (; currentNeuron != hiddenLayer.end(); ++currentNeuron) {
		output = currentNeuron->WeightedSum(inputValues);

		if (biasNode) {
			// All this bit does is pick the last value on the current 
			// neuron's vector of weights and subtract it from the
			// weighted sum:
			output += currentNeuron->weights.back();
			// The idea is that rather than insist on a neuron coming up with
			// outputs that are simply positive or negative, we might have a
			// neuron which produces a high valued output which we want to
			// tone down. So we subtract the bias value, which is just stored
			// at the end of the weights for convenience, from the neuron's
			// output. The end result is a much more flexible network.
		}
		
		// Now the value is processed by either a threshold function, which
		// will make our hard-computed output into nothing more than a 0 or 1,
		// or a sigmoid function which will make values <0 a bit lower, and
		// values >0 a bit higher. The net would work without this stage, but
		// not very well.
		output = ActivationFunction(output);

		// Now we store the outputs of each neuron ready to be passed onto the
		// next layer.
		hiddenOutput.push_back(output);
	}

	// Now we do exactly the same thing over again for the output layer. Note
	// that in this net we will never have more than one hidden layer, so I've
	// simply written out the firing code twice, once for each layer. Many-
	// layered ANN implementations would most likely do this bit as a loop,
	// with one iteration per layer.
	for (currentNeuron = outputLayer.begin();
		 currentNeuron != outputLayer.end(); ++currentNeuron) {
		output = currentNeuron->WeightedSum(hiddenOutput);

		if (biasNode) {
			output += currentNeuron->weights.back();
		}

		output = ActivationFunction(output);

		// These are the values we'll be giving to the user when they request
		// the output with GetOutput();
		outputValues.push_back(output);

	}
}

/**
 * The squashing function for the network, either a sigmoid or threshold
 * function.
 * \param n The input value.
 * \return The output value.
 */
float FeedForwardNet::ActivationFunction(float n)
{
	if (sigmoid) {
		// This is the sigmoid function, refer to the lecture notes or a good
		// book on ANNs for a better diagram than this.
		//
		//   1 |       .-'''	The sigmoid function is somewhere between an
		//     |      /     	identity function (i.e. returns whatever you
		//     |     /	   		put in) and a threshold (returns 0 for <= 0,
		//   0 |__.-'_______	1 for > 0). When plotted it's a smooth curve,
		//     -10    0   10	just like the one to the left.
		//
		return static_cast<float>(1.0 / (1.0 + exp(-n / FFN_ACTIVATION_RESPONSE)));
	}
	else {
		// This is just a threshold function. You'll want to use sigmoid for
		// most ANN applications, this is really more use for testing, e.g.
		// with the XOR problem (although of course that can be done with
		// sigmoid too)
		return n > 0.0f ? 1.0f : 0.0f;
	}
}

/**
 * Outputs all the data for the net to the specified output stream.
 * \param out The output stream.
 */
void FeedForwardNet::Serialise(ostream& out)const
{
	out << "FeedForwardNet\n"
		<< setprecision(36)
		<< inputs << endl
		<< outputs << endl
		<< hidden << endl
		<< (sigmoid ? "sigmoid" : "threshold") << endl
		<< (biasNode ? "biasnode" : "nobiasnode") << endl;

	ostream_iterator<float> outIter(out, "\n");
	copy(inputValues.begin(), inputValues.end(), outIter);
	copy(outputValues.begin(), outputValues.end(), outIter);

	vector<float> config = GetConfiguration();
	copy(config.begin(), config.end(), outIter);
}

/**
 * Reconstructs the network from an input stream.
 * \param in The input stream.
 */
void FeedForwardNet::Unserialise(istream& in)
{
	string name;
	in >> name;
	if (name != "FeedForwardNet") {
		throw SerialException(SERIAL_ERROR_WRONG_TYPE, name, 
							  "This object is type FeedForwardNet");
	}

	in >> inputs
	   >> outputs
	   >> hidden
	   >> switcher("sigmoid", sigmoid)
	   >> switcher("biasnode", biasNode);

	Init(inputs, outputs, hidden, sigmoid, biasNode); // Looks a little fishy

	copy_from_istream(inputValues.begin(), inputValues.end(), in);
	copy_from_istream(outputValues.begin(), outputValues.end(), in);

	vector<float> config(GetConfigurationLength());
	copy_from_istream(config.begin(), config.end(), in);

	SetConfiguration(config);
}

/**
 * Outputs the network's data in a pretty format.
 * \return An STL string.
 */
string FeedForwardNet::ToString()const
{
	// I'm not going to comment this because, well, it's just too boring.

	ostringstream out;
	vector<Neuron>::const_iterator i;
	vector<float>::const_iterator j;

	out << setprecision(2) << setiosflags(ios::fixed)
		<< "Input values:" << endl;

	for (j = inputValues.begin(); j != inputValues.end(); ++j) {
		out << setw(FFN_COLSIZE) << *j;
	}

	out << endl << endl
		<< "Hidden layer weights: " << endl;

	for (i = hiddenLayer.begin(); i != hiddenLayer.end(); ++i) {
		for (j = i->weights.begin(); j != i->weights.end() - 1; ++j) {
			out << setw(FFN_COLSIZE) << *j;
		}
		if (biasNode) out << " bias: ";
		out << setw(FFN_COLSIZE) << *j << endl;
	}

	out << endl
		<< "Output layer weights: " << endl;

	for (i = outputLayer.begin(); i != outputLayer.end(); ++i) {
		for (j = i->weights.begin(); j != i->weights.end() - 1; ++j) {
			out << setw(FFN_COLSIZE) << *j;
		}
		if (biasNode) out << " bias: ";
		out << setw(FFN_COLSIZE) << *j << endl;
	}

	out << endl
		<< "Output values: " << endl;

	for (j = outputValues.begin(); j != outputValues.end(); ++j) {
		out << setw(FFN_COLSIZE) << *j;
	}

	out << endl << endl;

	return out.str();
}

/**
 * Calculates the expected length of the configuration data for the network.
 * \return The configuration length.
 * \see FeedForwardNet::GetConfiguration
 */
int FeedForwardNet::GetConfigurationLength()const
{
	return hidden * inputs + outputs * hidden
		   + (biasNode ? 1 : 0) * (hidden + outputs);
}

/** 
 * Output operator overload for the FeedForwardNet.
 * \param out An output stream.
 * \param ffn A FeedForwardNet object.
 * \return A reference to the output stream.
 * \see FeedForwardNet::Serialise
 */ 
ostream& operator<< (ostream& out, const FeedForwardNet& ffn)
{
	ffn.Serialise(out);
	return out;
}

/** 
 * Input operator overload for the FeedForwardNet.
 * \param in An input stream.
 * \param ffn A FeedForwardNet object.
 * \return A reference to the input stream.
 * \see FeedForwardNet::Unserialise
 */ 
istream& operator>> (istream& in, FeedForwardNet& ffn)
{
	ffn.Unserialise(in);
	return in;
}

} // namespace BEAST
