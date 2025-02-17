
import os
import argparse
import json

from mltk.core.model import load_tflite_model
from mltk.core.tflite_model import TfliteModel
from mltk.core.tflite_micro import TfliteMicro
from mltk.utils.bin2header import bin2header
from mltk.utils.path import fullpath, create_tempdir
from mltk.utils.hasher import hash_file
from mltk import cli


def generate_model_header(
    model: str,
    output: str,
    variable_name='MODEL_DATA',
    variable_attributes:str=None,
    length_variable_name='MODEL_DATA_LENGTH',
    accelerator:str=None
):
    """Generate a model header file from a MLTK model or .tflite
    
    Args:
        model: Name of MLTK model or path to .tflite
        output: Path to generated output header
        variable_name: Name of C array
        variable_attributes: Attributes to prepend to C array variable
        length_variable_name: Name of C variable to hold length of C array
        accelerator: Name of accelerator for which to generate header
    """

    try:
        tflite_path = load_tflite_model(
            model, 
            print_not_found_err=True, 
            return_tflite_path=True
        )
    except Exception as e:
        cli.abort(msg=f'\n\nFailed to load tflite model, err: {e}\n\n')

    output = fullpath(output)
    old_generation_details = None
    generation_args_path = f'{os.path.dirname(output)}/generated_model_details.json'
    generation_details = dict(
        tflite_path=fullpath(tflite_path),
        tflite_hash=hash_file(tflite_path),
        output=output,
        accelerator=accelerator
    )
    if os.path.exists(generation_args_path):
        try:
            with open(generation_args_path, 'r') as f:
                old_generation_details = json.load(f)
        except:
            pass 

    if old_generation_details == generation_details:
        print(f'{os.path.basename(output)} up-to-date')
        return 



    if accelerator:
        if not TfliteMicro.accelerator_is_supported(accelerator):
            raise ValueError(f'Unknown accelerator: {accelerator}, supported accelerators are: {", ".join(TfliteMicro.get_supported_accelerators())}')

        tflm_accelerator = TfliteMicro.get_accelerator(accelerator)
        if tflm_accelerator.supports_model_compilation:
            compilation_report_path = output + '-compilation_report.txt'
            tflite_model = TfliteModel.load_flatbuffer_file(tflite_path)
            compiled_tflite_model = tflm_accelerator.compile_model(
                tflite_model, 
                report_path=compilation_report_path,
                logger=cli.get_logger()
            )
            model_name = os.path.basename(tflite_path)[:-len('.tflite')]
            tflite_path = f'{create_tempdir("tmp_models")}/{model_name}.{accelerator}.tflite'
            compiled_tflite_model.save(tflite_path)


    bin2header(
        input=tflite_path, 
        output_path=output, 
        var_name=variable_name,
        length_var_name=length_variable_name,
        attributes=variable_attributes,
    )
   
    with open(generation_args_path, 'w') as f:
        json.dump(generation_details, f, indent=3)

    cli.print_info(f'Generated {output}\nfrom {tflite_path}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate a model header file from a MLTK model or .tflite')
    parser.add_argument('model', help='Name of MLTK model or path to .tflite')
    parser.add_argument('--output', default='generated_model.tflite.h', help='Path to generated output header')
    parser.add_argument('--name', default='MODEL_DATA', help='Name of C array')
    parser.add_argument('--length_name', default='MODEL_DATA_LENGTH', help='Name of C variable to hold length of data in bytes')
    parser.add_argument('--attributes', default=None, help='Attributes to prepend to C array variable')
    parser.add_argument('--accelerator', default=None, help='Specific accelerator for which to generate model header')

    args = parser.parse_args()
    try:
        generate_model_header(
            model=args.model,
            output=args.output, 
            variable_name=args.name,
            variable_attributes=args.attributes,
            length_variable_name=args.length_name,
            accelerator=args.accelerator
        )
    except Exception as _ex:
        cli.handle_exception('Failed to generate model header', _ex)