from typing import List
import tensorflow_lite_support.metadata.schema_py_generated as tflite_fb
from tensorflow_lite_support.metadata.schema_py_generated import flatbuffers

from .tflite_layer_generator import (
    TfliteOpCode,
    TfliteLayerGeneratorConfig,
    TfliteTensorGenerator, 
    TfliteLayerGenerator, 
    opcode_to_layer_generator_class
)


class TfliteModelGenerator:

    def __init__(self):
        self.layers:List[TfliteLayerGenerator] = []


    @property
    def model_inputs(self) -> List[TfliteTensorGenerator]:
        retval:List[TfliteTensorGenerator] = []
        for layer in self.layers:
            for input_tensor in layer.inputs:
                if input_tensor.is_model_input:
                    retval.append(input_tensor)
        return retval


    def add_layer_with_config(
        self, 
        opcode:TfliteOpCode, 
        config:TfliteLayerGeneratorConfig
    ) -> TfliteLayerGenerator:
        layer_class = opcode_to_layer_generator_class(opcode)
        if layer_class is None:
            raise ValueError(f'Unsupported opcode: {opcode}')
        if isinstance(config, dict):
            config = TfliteLayerGeneratorConfig(config)
        layer = layer_class(config)
        layer._index = len(self.layers)
        self.layers.append(layer)
        return layer


    def generate(self) -> bytes:
        operators:List[tflite_fb.OperatorT] = []
        opcodes:List[tflite_fb.OperatorCodeT] = []
        buffers:List[tflite_fb.BufferT] = []
        tensors:List[tflite_fb.TensorT] = []
        model_inputs:List[int] = []
        model_outputs:List[int] = []

        null_buffer = tflite_fb.BufferT()
        null_buffer.data = None
        buffers.append(null_buffer)

        for layer in self.layers:
            operator = tflite_fb.OperatorT()
            operators.append(operator)

            opcode_exists = False
            for i, op in enumerate(opcodes):
                if layer.opcode == max(op.deprecatedBuiltinCode, op.builtinCode):
                    operator.opcodeIndex = i 
                    opcode_exists = True
                    break

            if not opcode_exists:
                operator.opcodeIndex = len(opcodes)
                opcode = tflite_fb.OperatorCodeT()
                # For mode details on what's going on here, see:
                # https://github.com/tensorflow/community/pull/285/files
                if layer.opcode > 127:
                    opcode.builtinCode = layer.opcode
                else:
                    opcode.deprecatedBuiltinCode = layer.opcode
                
                opcodes.append(opcode)

            operator.builtinOptionsType, operator.builtinOptions = layer.generate_options()

            operator.inputs = []
            operator.outputs = []
            for layer_input in layer.inputs:
                tensor_index = len(tensors)

                tensor = tflite_fb.TensorT()
                tensor.name = layer_input.name
                tensor.shape = layer_input.shape
                tensor.type = layer_input.tflite_type

                if layer_input.data is None or layer_input.is_model_input:
                    # FIX ME: currently, each layer's data input is an input to the model
                    # So basically, each layer is parallel, they don't connect to each other as a sequence
                    model_inputs.append(tensor_index)
                    tensor.buffer = 0
                else:
                    buffer = tflite_fb.BufferT()
                    buffer.data = layer_input.data.tobytes()
                    tensor.buffer = len(buffers)
                    buffers.append(buffer)

                quantization = tflite_fb.QuantizationParametersT()
                quantization.zeroPoint = layer_input.quantization.zeropoint
                quantization.scale = layer_input.quantization.scale
                quantization.quantizedDimension = layer_input.quantization.quantization_dimension
                quantization.min = []
                quantization.max = []
                self.detailsType = 1
                tensor.quantization = quantization
    
                operator.inputs.append(tensor_index)
                tensors.append(tensor)

            for layer_output in layer.outputs:
                tensor_index = len(tensors)

                tensor = tflite_fb.TensorT()
                tensor.name = layer_output.name
                tensor.shape = layer_output.shape
                tensor.type = layer_output.tflite_type
                tensor.buffer = 0

                quantization = tflite_fb.QuantizationParametersT()
                quantization.zeroPoint = layer_output.quantization.zeropoint
                quantization.scale = layer_output.quantization.scale
                quantization.quantizedDimension = layer_output.quantization.quantization_dimension
                quantization.min = []
                quantization.max = []
                self.detailsType = 1
                tensor.quantization = quantization
    
                operator.outputs.append(tensor_index)
                # FIX ME: currently, each layer's output is an output to the model
                model_outputs.append(tensor_index)
                tensors.append(tensor)

    
        subgraph = tflite_fb.SubGraphT()
        subgraph.tensors = tensors
        subgraph.inputs = model_inputs
        subgraph.outputs = model_outputs
        subgraph.operators = operators
        subgraph.name = 'subgraph0'

        model = tflite_fb.ModelT()
        model.version = 3 # .tflite schema version 3
        model.description = 'Generated by TfliteModelGenerator'
        model.subgraphs = [subgraph]
        model.buffers = buffers
        model.operatorCodes = opcodes

        fbb = flatbuffers.Builder(1024*1024)
        model_offset = model.Pack(fbb)

        fbb.Finish(model_offset, b"TFL3")
        return bytes(fbb.Output())