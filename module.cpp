#include <opennn/opennn.h>
#include "ylang/vm.hpp"
#include <iostream>

// exposes the protected epoch limit
struct OptimizerHack : public opennn::Optimizer {
	static void set_epochs(opennn::Optimizer *opt, long epochs) {
		static_cast<OptimizerHack *>(opt)->maximum_epochs = (Index)epochs;
	}
};

extern "C" void YlangLoadModule(std::shared_ptr<Env> env, const std::vector<std::string> &symbols) {
    auto moduleNamespace = std::make_shared<ClassObject>("OpenNN");
		moduleNamespace->mro.push_back(moduleNamespace.get());
		auto defineClass = [&](std::shared_ptr<ClassObject> cls) {
			Value classVal;
			classVal.type = ValueType::CLASS;
			classVal.ref = cls;
			moduleNamespace->staticFields[cls->name] = classVal;
			if (symbols.size() == 1 && symbols[0] == "*") {
				env->set(cls->name, classVal, true, true);
				return;
			}
			for (const auto &s : symbols) {
				if (s == cls->name) {
					env->set(cls->name, classVal, true, true);
					break;
				}
			}
		};
		static auto nnClass = std::make_shared<ClassObject>("NeuralNetwork");
      nnClass->staticFields["APPROXIMATION"] = Value::Int(0);
      nnClass->staticFields["CLASSIFICATION"] = Value::Int(1);
		nnClass->methods["__construct__"] = ClassObject::MethodInfo{
			.func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
				if (args.size() < 5) throw ArgumentError("NeuralNetwork(inputs, outputs, hidden, [mode])", l, c);
				long inputs = (long)args[2].asInt();
				long outputs = (long)args[3].asInt();
				long hidden = (long)args[4].asInt();
            int mode = 0;
            if (args.size() >= 6) mode = (int)args[5].asInt();
            opennn::NeuralNetwork* net = nullptr;
            if (mode == 1) net = new opennn::ClassificationNetwork({inputs}, {hidden}, {outputs});
            else net = new opennn::ApproximationNetwork({inputs}, {hidden}, {outputs});
            auto *self = static_cast<InstanceObject *>(args[0].ref.get());
            self->fields["ptr"] = Value::pInt(net);
				return args[0];
			}),
			.access = AccessLevel::PUBLIC
		};
		nnClass->methods["__destruct__"] = ClassObject::MethodInfo{
			.func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
				auto *self = static_cast<InstanceObject *>(args[0].ref.get());
				if (self->fields.find("ptr") != self->fields.end()) {
					auto *net = (opennn::NeuralNetwork *)self->fields["ptr"].aspInt();
					delete net;
				}
				return Value::None();
			}),
			.access = AccessLevel::PUBLIC
		};
		nnClass->methods["predict"] = ClassObject::MethodInfo{
         .func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
            if (args.size() < 3) throw ArgumentError("predict() requires 1 argument", l, c);
            auto *self = static_cast<InstanceObject *>(args[0].ref.get());
            auto *net = (opennn::NeuralNetwork *)self->fields["ptr"].aspInt();
            std::vector<Value>* elements_ptr = nullptr;
            if (args[2].type == ValueType::VECTOR) elements_ptr = &static_cast<VectorObject *>(args[2].ref.get())->elements;
				else if (args[2].type == ValueType::LIST) elements_ptr = &static_cast<ListObject *>(args[2].ref.get())->elements;
				else if (args[2].type == ValueType::TUPLE) elements_ptr = &static_cast<TupleObject *>(args[2].ref.get())->elements;
				else throw ArgumentError("predict() expects a vector <>, list [], or tuple ()", l, c);
            long input_size = (long)elements_ptr->size();
            MatrixR inputs(1, input_size);
            for (long i = 0; i < input_size; i++) inputs(0, i) = (*elements_ptr)[i].asFloat();
            MatrixR outputs = net->calculate_outputs(inputs);
            std::vector<Value> result;
            long output_size = outputs.cols();
            for (long i = 0; i < output_size; i++) result.push_back(Value::Float(outputs(0, i)));
				return Value::Vector(result);
         }),
         .access = AccessLevel::PUBLIC
      };
		nnClass->methods["get_weights"] = ClassObject::MethodInfo{
			.func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
				if (args.size() > 2) throw ArgumentError("get_weights()", l, c);
				auto *self = static_cast<InstanceObject *>(args[0].ref.get());
				auto *net = (opennn::NeuralNetwork *)self->fields["ptr"].aspInt();
				auto arr = net->get_parameters();
            std::vector<Value> vec(arr.size());
            for (const auto& element : arr) vec.push_back(Value::Float(element));
            Value resArr = Value::Vector(vec);
				return resArr;
			}),
			.access = AccessLevel::PUBLIC
		};
      nnClass->methods["set_weights"] = ClassObject::MethodInfo{
			.func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
				if (args.size() != 3) throw ArgumentError("set_weights(vector)", l, c);
				auto *self = static_cast<InstanceObject *>(args[0].ref.get());
				auto *net = (opennn::NeuralNetwork *)self->fields["ptr"].aspInt();
				auto *weights = static_cast<VectorObject *>(args[2].ref.get());
            VectorR w(weights->elements.size());
            for (size_t i = 0; i < w.size(); i++) w[i] = weights->elements[i].asFloat();
            net->set_parameters(w);
				return Value::None();
			}),
			.access = AccessLevel::PUBLIC
		};
      nnClass->methods["save"] = ClassObject::MethodInfo{
			.func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
				if (args.size() < 3) throw ArgumentError("save(filepath)", l, c);
				auto *self = static_cast<InstanceObject *>(args[0].ref.get());
				auto *net = (opennn::NeuralNetwork *)self->fields["ptr"].aspInt();
				net->save(args[2].asString());
				return Value::None();
			}),
			.access = AccessLevel::PUBLIC
		};
		nnClass->methods["load"] = ClassObject::MethodInfo{
			.func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
				if (args.size() < 3 || args[2].type != ValueType::STRING) throw ArgumentError("load(filepath)", l, c);
				auto *self = static_cast<InstanceObject *>(args[0].ref.get());
				auto *net = (opennn::ApproximationNetwork *)self->fields["ptr"].aspInt();
				net->load(args[2].asString());
				return Value::None();
			}),
			.access = AccessLevel::PUBLIC
		};
		static auto dataClass = std::make_shared<ClassObject>("Dataset");
		dataClass->methods["__construct__"] = ClassObject::MethodInfo{
			.func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
				if (args.size() < 4 || args[2].type != ValueType::STRING) throw ArgumentError("Dataset(csv_filepath_string, has_header_bool)", l, c);
				bool has_header = args[3].asBool();
				auto *ds = new opennn::Dataset(args[2].asString(), ",", has_header, false);
				auto *self = static_cast<InstanceObject *>(args[0].ref.get());
				self->fields["ptr"] = Value::pInt(ds);
				return args[0];
			}),
			.access = AccessLevel::PUBLIC
		};
		dataClass->methods["__destruct__"] = ClassObject::MethodInfo{
			.func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
				auto *self = static_cast<InstanceObject *>(args[0].ref.get());
				if (self->fields.find("ptr") != self->fields.end()) {
					auto *ds = (opennn::Dataset *)self->fields["ptr"].aspInt();
					delete ds;
				}
				return Value::None();
			}),
			.access = AccessLevel::PUBLIC
		};
      dataClass->methods["split"] = ClassObject::MethodInfo{
         .func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
            if (args.size() < 5) throw ArgumentError("split(train_pct, val_pct, test_pct)", l, c);
            auto *self = static_cast<InstanceObject *>(args[0].ref.get());
            auto *ds = (opennn::Dataset *)self->fields["ptr"].aspInt();
            ds->split_samples_random(args[2].asFloat(), args[3].asFloat(), args[4].asFloat());
            return Value::None();
         }),
         .access = AccessLevel::PUBLIC
      };
      dataClass->methods["scale_inputs"] = ClassObject::MethodInfo{
         .func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
            auto *self = static_cast<InstanceObject *>(args[0].ref.get());
            auto *ds = (opennn::Dataset *)self->fields["ptr"].aspInt();
            ds->scale_features("Input");
            return Value::None();
         }),
         .access = AccessLevel::PUBLIC
      };
		dataClass->methods["set_columns"] = ClassObject::MethodInfo{
			.func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
				if (args.size() < 4) throw ArgumentError("set_columns(input_count, target_count)", l, c);
				auto *self = static_cast<InstanceObject *>(args[0].ref.get());
				auto *ds = (opennn::Dataset *)self->fields["ptr"].aspInt();
				long inputs = (long)args[2].asInt();
				long targets = (long)args[3].asInt();
				long total_vars = (long)ds->get_variables().size();
            for (long i = 0; i < total_vars; i++) {
               if (i < inputs) ds->set_variable_role(i, "Input");
					else if (i < inputs + targets) ds->set_variable_role(i, "Target");
					else ds->set_variable_role(i, "Unused");
            }
            return Value::None();
			}),
			.access = AccessLevel::PUBLIC
		};
		static auto optClass = std::make_shared<ClassObject>("Optimizer");
		optClass->staticFields["GRADIENT_DESCENT"] = Value::String("StochasticGradientDescent");
		optClass->staticFields["QUASI_NEWTON"] = Value::String("QuasiNewtonMethod");
		optClass->staticFields["LEVENBERG_MARQUARDT"] = Value::String("LevenbergMarquardtAlgorithm");
		optClass->staticFields["ADAM"] = Value::String("AdaptiveMomentEstimation");
      optClass->staticFields["MEAN_SQUARED_ERROR"] = Value::String("MeanSquaredError");
      optClass->staticFields["NORMALIZED_SQUARED_ERROR"] = Value::String("NormalizedSquaredError");
      optClass->staticFields["CROSS_ENTROPY_ERROR"] = Value::String("CrossEntropyError");
		optClass->methods["__construct__"] = ClassObject::MethodInfo{
			.func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
				if (args.size() < 4) throw ArgumentError("Optimizer(network_instance, dataset_instance)", l, c);
				auto *self = static_cast<InstanceObject *>(args[0].ref.get());
				auto *netInst = static_cast<InstanceObject *>(args[2].ref.get());
				auto *dataInst = static_cast<InstanceObject *>(args[3].ref.get());
				auto *net = (opennn::NeuralNetwork *)netInst->fields["ptr"].aspInt();
				auto *ds = (opennn::Dataset *)dataInst->fields["ptr"].aspInt();
				auto *strategy = new opennn::TrainingStrategy(net, ds);
				self->fields["ptr"] = Value::pInt(strategy);
				return args[0]; 
			}),
			.access = AccessLevel::PUBLIC
		};
      optClass->methods["set_learning_rate"] = ClassObject::MethodInfo{
         .func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
            if (args.size() < 3) throw ArgumentError("set_learning_rate(float)", l, c);
            auto *self = static_cast<InstanceObject *>(args[0].ref.get());
            auto *strategy = (opennn::TrainingStrategy *)self->fields["ptr"].aspInt();
            opennn::Optimizer* opt = strategy->get_optimization_algorithm();
            double lr = args[2].asFloat();
            if (auto adam = dynamic_cast<opennn::AdaptiveMomentEstimation *>(opt)) {
                  adam->set_learning_rate(lr);
            }
            else if (auto gd = dynamic_cast<opennn::StochasticGradientDescent *>(opt)) {
                  gd->set_initial_learning_rate(lr);
            }
            else {
                  throw RuntimeError("Current optimizer does not support manual learning rates!", l, c);
            }
            return Value::None();
         }),
         .access = AccessLevel::PUBLIC
      };
      optClass->methods["set_loss"] = ClassObject::MethodInfo{
         .func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
            if (args.size() < 3 || args[2].type != ValueType::STRING) throw ArgumentError("set_loss(string)", l, c);
            auto *self = static_cast<InstanceObject *>(args[0].ref.get());
            auto *strategy = (opennn::TrainingStrategy *)self->fields["ptr"].aspInt();
            strategy->set_loss(args[2].asString());
            return Value::None();
         }),
         .access = AccessLevel::PUBLIC
      };
		optClass->methods["set_epochs"] = ClassObject::MethodInfo{
			.func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
				if (args.size() < 3) throw ArgumentError("set_epochs(int)", l, c);
				auto *self = static_cast<InstanceObject *>(args[0].ref.get());
				auto *strategy = (opennn::TrainingStrategy *)self->fields["ptr"].aspInt();
				long epochs = (long)args[2].asInt();
				OptimizerHack::set_epochs(strategy->get_optimization_algorithm(), epochs);
				return Value::None();
			}),
			.access = AccessLevel::PUBLIC
		};
		optClass->methods["train"] = ClassObject::MethodInfo{
			.func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
				auto *self = static_cast<InstanceObject *>(args[0].ref.get());
				auto *strategy = (opennn::TrainingStrategy *)self->fields["ptr"].aspInt();
				auto results = strategy->train();
				return Value::Float((double)results.get_training_error());
			}),
			.access = AccessLevel::PUBLIC
		};
      optClass->methods["train_with_history"] = ClassObject::MethodInfo{
         .func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
            auto *self = static_cast<InstanceObject *>(args[0].ref.get());
            auto *strategy = (opennn::TrainingStrategy *)self->fields["ptr"].aspInt();
            auto results = strategy->train();
            std::vector<Value> history_vec;
            long hist_size = (long)results.training_error_history.size();
            for (long i = 0; i < hist_size; i++) history_vec.push_back(Value::Float((double)results.training_error_history[i]));
            return Value::Vector(history_vec);
         }),
         .access = AccessLevel::PUBLIC
      };
		optClass->methods["set_algorithm"] = ClassObject::MethodInfo{
			.func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
				if (args.size() < 3 || args[2].type != ValueType::STRING) throw ArgumentError("set_algorithm(string)", l, c);
				auto *self = static_cast<InstanceObject *>(args[0].ref.get());
				auto *strategy = (opennn::TrainingStrategy *)self->fields["ptr"].aspInt();
				strategy->set_optimization_algorithm(args[2].asString());
				return Value::None();
			}),
			.access = AccessLevel::PUBLIC
		};
		optClass->methods["__destruct__"] = ClassObject::MethodInfo{
			.func = Value::Native([](const std::vector<Value> &args, int l, int c) -> Value {
				auto *self = static_cast<InstanceObject *>(args[0].ref.get());
				if (self->fields.find("ptr") != self->fields.end()) {
					auto *strategy = (opennn::TrainingStrategy *)self->fields["ptr"].aspInt();
					delete strategy;
				}
				return Value::None();
			}),
			.access = AccessLevel::PUBLIC
		};
		defineClass(nnClass);
		defineClass(dataClass);
		defineClass(optClass);
	}
