/**
 * \file dynamicalnet.h
 * This file contains the interface for the DynamicalNet object, a
 * fully-recurrent, continuous-time neural network.
 * \author David Gordon
 * \addtogroup biosystems
 */

#ifndef _DYNAMICALNET_H
#define _DYNAMICALNET_H

#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <iterator>
#include <functional>
#include <numeric>
#include <iomanip>
#include <cmath>

#include "random.h"
#include "serialfuncs.h"

namespace BEAST {

/**
 * \addtogroup biosystems
 * @{
 */

/**
 * This class implements a fully recurrent continuous (or dynamical) neural
 * network. The network is configured with a number of nodes, some or all of
 * which may also act as input nodes, and some or all of which may act as
 * output nodes. Every node on firing takes a weighted sum of the activation
 * states of every other node, including itself. This approach allows the
 * network to store information and perform far more complex tasks than a
 * feed-forward net might.
 * The actual design of dynamical networks has many interpretations, but for
 * reference, this one corresponds as closely as possible to the network
 * described in Yamauchi, B. M., & Beer, R. D. (1994). Sequential behavior and
 * learning in evolved dynamical neural networks. Adaptive Behavior 2(3), 
 * 219--246. http://citeseer.nj.nec.com/yamauchi94sequential.html
 * \see FeedForwardNet
 */
class DynamicalNet
{
protected:
	/**
	 * Unlike the FeedForwardNet, the Neuron in DynamicalNet is more worthy of
	 * its name, since nearly all the processing of the DNN's firing algorithm
	 * occurs here. Each Neuron contains a weight for each other neuron (and
	 * one for itself), one or multiple input channels and one or multiple
	 * output channels, with a series of weights in each case if multiple
	 * channels are in use. The neuron also keeps track of its activation
	 * value and its output value, which is simply the biased, squashed
	 * activation value.
	 * \see FeedForwardNet::Neuron
	 */
	struct Neuron {
		/**
		 * Constructor: sets the number of inputs, outputs and weights for
		 * this neuron, as well as input and output channels.
		 * \param i The number of input weights.
		 * \param o The number of output weights.
		 * \param t The total number of internal weights.
		 * \param inCh The number of input channels.
		 * \param outCh The number of output channels.
		 * \param p A pointer to the parent DynamicalNet
		 */
		Neuron(int i, int o, int t, int inCh, int outCh, DynamicalNet* p):
			inputChannel(inCh), outputChannel(outCh), 
			output(0.0f), activation(0.0f), inputWeights(i), outputWeights(o),
			weights(t + 2), parent(p), bias(0.0f), timeConstant(1.0f){}

		void				Randomise();

		void				Fire();

		float				GetOutput()const { return output; }

		void				GetConfiguration(std::vector<float>& config)const;
		std::vector<float>::const_iterator
							SetConfiguration(std::vector<float>::const_iterator config);

		std::string			ToString()const;
							
		int inputChannel;		///< This neuron's input channel, or -1 for all.
		int outputChannel;		///< This neuron's output channel, or -1 for all.

		float output;			///< The neuron's current output.
		float activation;		///< The neuron's current activation.

		std::vector<float>	inputWeights;	///< The input weights.
		std::vector<float>	outputWeights;	///< The output weights.
		std::vector<float>	weights;		///< Weights for every neuron, bias and time constant.

		DynamicalNet* parent;	///< The net this neuron belongs to.

		float bias;				///< The bias, taken from weights.end() - 2
		float timeConstant;		///< Taken from weights.end() - 1

		/// Returns a random number in the range [-1,1]
		static float	RandomNum() { return randval(2.0f) - 1.0f; }

		/// The standard ANN squashing function
		static float	Sigmoid(float y)
		{
			return static_cast<float>(1.0 / (1.0 + exp(static_cast<double>(-y))));
		}
	};

	friend struct Neuron;

public:
	DynamicalNet(int inputs, int outputs, int total,
		bool multiInputNodes = true, bool multiOutputNodes = false);
	~DynamicalNet();

	void			Init(int inputs, int outputs, int total, 
						 bool multIn, bool multOut);

	void			Reset();

	void			SetInputChannel(int neuron, int channel);
	void			SetOutputChannel(int neuron, int channel); 

	void			Randomise();

	/// Sets the input value for channel n to f.
	void			SetInput(int n, float f)		{ inputs[n] = f; }

	/// Sets all the input values at once from a vector of floats.
	void			SetInput(const std::vector<float>& v){ inputs = v; }

	/// Returns the output value for the specified output channel.
	float			GetOutput(int n)const			{ return outputs[n]; }

	/// Returns all the outputs at once as a vector of floats.
	const std::vector<float>& GetOutputs()const		{ return outputs; }

	void			Fire();

	void			SetConfiguration(const std::vector<float>&);
	std::vector<float>	GetConfiguration()const;

	std::string		ToString()const;

	void			Serialise(std::ostream&)const;
	void			Unserialise(std::istream&);

protected:
	std::vector<Neuron>&	GetNeurons()			{ return neurons; }
	std::vector<float>&		GetInputs()				{ return inputs; }
	std::vector<float>&		GetOutputs()			{ return outputs; }
	std::vector<float>&		GetNeuronStates()		{ return neuronStates; }

	bool					IsMultiInputNodes()		{ return multiInputNodes; }
	bool					IsMultiOutputNodes()	{ return multiOutputNodes; }

private:
	std::vector<Neuron>		neurons;	///< A vector of the net's neurons.

	std::vector<float>		inputs;		///< The current input values.
	std::vector<float>		outputs;	///< The current output values.
	std::vector<float>		neuronStates;	///< The current neuron activation states.

	bool multiInputNodes;	///< True if there are multiple input nodes per channel.
	bool multiOutputNodes;	///< True if there are multiple output nodes pre channel.

	int						GetConfigurationLength()const;
};

std::ostream& operator<<(std::ostream& out, const DynamicalNet& dnn);
std::istream& operator>>(std::istream& in, DynamicalNet& dnn);

/**
 * @}
 */

} // namespace BEAST

#endif
