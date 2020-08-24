/**
 * \file feedforwardnet.h
 * \author David Gordon
 */

#ifndef FEEDFORWARD_NET
#define FEEDFORWARD_NET

#include <vector>
#include <string>	
#include <algorithm>// for_each, generate
#include <cmath>	// exp used in sigmoid
#include <iostream>	// Printed output
#include <sstream>	// Import and export of configuration
#include <iomanip>	// Formats output
#include <numeric>	// For inner_product
#include <iterator> // For istream_iterator and ostream_iterator

#include "random.h"	// For FeedForwardNet::Randomise
#include "serialfuncs.h" // For FeedFowardNet::Serialise and FeedForwardNet::Unserialise

namespace BEAST { 

/**
 * \addtogroup biosystems
 * @{
 */

/// This value decides the curve of the sigmoid function.
const double FFN_ACTIVATION_RESPONSE = 1;
/// The width of columns in ToString output.
const int FFN_COLSIZE = 6;

/**
 * This is an implementation of a simple two-layer feed-forward neural
 * network. You may specify the number of inputs, the number of nodes in the
 * output layer and the number of nodes in the hidden layer. Every node has an
 * associated bias term, although this can be switched off. The default
 * activation function is sigmoid, although it may be swtiched off and is then
 * replaced by a simple threshold function. It is also possible to configure a
 * FeedForwardNet with a hidden layer size of 0, in which case the net becomes
 * a perceptron, bypassing the hidden layer entirely.
 * \see DynamicalNet
 */
class FeedForwardNet
{
protected:
	/**
	 * This member struct simply encapsulates the weighted sum function which
	 * has to be performed on the weights of each node when the net fires.
	 * Note that Neuron is an inaccurate term for this class since it doesn't
	 * actually do everything a neuron in a "real" net might - biasing and 
	 * squashing (via the activation function) occur outside Neuron, in the
	 * net's main firing function. This has been done for optimisation.
	 * \see Fire
	 * \see ActivationFunction
	 * \see DynamicalNet::Neuron
	 */
	struct Neuron
	{
		/// Constructor, simply specifies the number of weights for the Neuron.
		Neuron(int n):weights(n){}
		/// Constructor, initialises the neuron with a vector containing its weights.
		Neuron(std::vector<float> w):weights(w){}

		float				WeightedSum(std::vector<float>&)const;

		/// The weight values for this neuron (including bias).
		std::vector<float>	weights; 
	};

public:
	FeedForwardNet(int inputs, int outputs, int hidden = 0, 
				   bool sig = true, bool bias = true);
	virtual ~FeedForwardNet();

	void			Init(int in, int out, int hid, bool sig, bool bias);

	void			Randomise();

	/**
	 * Sets the current value of the specified input.
	 * \param n The number of the input to set.
	 * \param f The value to set it to.
	 * \warning This method does not check if the input specified exists, if
	 * you are unsure, check using GetInputs first.
	 */
	void			SetInput(int n, float f)			{ inputValues[n] = f; }
	void			SetInput(const std::vector<float>& v){ inputValues = v; }

	/**
	 * Returns the current value of the specified output.
	 * \param n The number of the output to return.
	 * \warning This method does not check if the output specified exists, if
	 * you are unsure, check using GetOutputs first.
	 */
	float			GetOutput(int n)const			{ return outputValues[n];}
	/**
	 * Returns the outputs of the neural net as a vector of floats.
	 */
	const std::vector<float>& GetOutput()const			{ return outputValues; }

	void			Fire();

	void			SetConfiguration(const std::vector<float>&);
	std::vector<float>	GetConfiguration()const;

	std::string		ToString()const;

	/// Returns the number of inputs on this network.
	int				GetInputs()const	{ return inputs; }
	/// Returns the number of outputs on this network.
	int				GetOutputs()const	{ return outputs; }
	/// Returns the number of hidden nodes on this network.
	int				GetHidden()const	{ return hidden; }
	/// Returns true if a sigmoid activation function is in use.
	bool			IsSigmoid()const	{ return sigmoid; }
	/// Returns true if a bias term is added to each node.
	bool			IsBiasNode()const	{ return biasNode; }
	
	void			Serialise(std::ostream&)const;
	void			Unserialise(std::istream&);

protected:
	std::vector<float>& GetInputValues()	{ return inputValues; }
	std::vector<float>& GetOutputValues()	{ return outputValues; }
	std::vector<Neuron>& GetHiddenLayer()	{ return hiddenLayer; }
	std::vector<Neuron>& GetOutputLayer()	{ return outputLayer; }

	float			ActivationFunction(float n);
	int				GetConfigurationLength()const;
	inline static float	RandomNum();

private:
	int				inputs;			///< The number of inputs recieved by the net.
	int				outputs;		///< The number of outputs the net produces.
	int				hidden;			///< The number of nodes in the hidden layer.

	bool			sigmoid;		///< Set to true (default) if a sigmoid ActivationFunction is used.
	bool			biasNode;		///< Set to true (default) if each node is to have its own bias term.

	std::vector<float>	inputValues;	///< A vector storing the current input values.
	std::vector<float>	outputValues;	///< A vector storing the current output values.
	std::vector<Neuron>	hiddenLayer;	///< A vector of hidden layer neurons.
	std::vector<Neuron>	outputLayer;	///< A vector of output layer neurons.
};

std::ostream& operator<< (std::ostream& out, const FeedForwardNet& ffn);
std::istream& operator>> (std::istream& in, FeedForwardNet& ffn);

/// Returns a number between -1 and 1
float FeedForwardNet::RandomNum()
{
	return randval(2.0f) - 1.0f;
}

/**
 * @}
 */

} // namespace BEAST

#endif
