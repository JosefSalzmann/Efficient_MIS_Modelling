#include "ann_sis_transfer_function.h"

#include <iostream>

void ANNSISTransferFunction::ReadModel(const std::string& file_name) {
	try {
		auto test_input = cppflow::tensor({0.0f, 0.0f, 0.0f});
		steepness_model = std::make_unique<cppflow::model>(cppflow::model(file_name + "_steepness"));
		shift_model = std::make_unique<cppflow::model>(cppflow::model(file_name + "_shift"));

		auto test_output_st = (*steepness_model)({{"serving_default_normalization_input:0", test_input}}, {"StatefulPartitionedCall:0"});
		auto test_output_sh = (*shift_model)({{"serving_default_normalization_input:0", test_input}}, {"StatefulPartitionedCall:0"});

		test_output_st.back().get_data<float>().back();
		test_output_sh.back().get_data<float>().back();

	} catch (const std::exception& e) {
		std::cerr << "Error whille loading ANN model: " << file_name << std::endl;
		throw std::exception();
	}
}

TransitionParameters ANNSISTransferFunction::CalculatePropagation(const std::vector<TransitionParameters>& parameters) {
	TransitionParameters current_inp_tr = parameters[0];
	TransitionParameters prev_outp_tr;

	// TODO: maybe remove this check and change method signature since we always get two transitions
	if (parameters.size() == 1) {
		prev_outp_tr = default_prev_tr;
	} else {
		prev_outp_tr = parameters[1];
	}

	double current_inp_steep = current_inp_tr.steepness;
	double prev_out_steep = prev_outp_tr.steepness;
	double time_shift = current_inp_tr.shift - prev_outp_tr.shift;

	if (time_shift > MAX_TIME_SHIFT) {
		time_shift = MAX_TIME_SHIFT;
	}

	auto ann_input = cppflow::tensor({(float)time_shift, (float)current_inp_steep, (float)prev_out_steep});
	auto ann_output_steepness = (*steepness_model)({{"serving_default_normalization_input:0", ann_input}}, {"StatefulPartitionedCall:0"});
	auto ann_output_shift = (*shift_model)({{"serving_default_normalization_input:0", ann_input}}, {"StatefulPartitionedCall:0"});

	double output_steepness = (double)ann_output_steepness.back().get_data<float>().back();
	double output_shift = (double)ann_output_shift.back().get_data<float>().back();

	output_shift = (output_shift < 0) ? 0 : output_shift;

	return {output_steepness, current_inp_tr.shift + output_shift};
}
void ANNSISTransferFunction::SetDefaultValues(const TransitionParameters& default_prev_transition, double maximal_shift) {
	MAX_TIME_SHIFT = maximal_shift;
	default_prev_tr = default_prev_transition;
}