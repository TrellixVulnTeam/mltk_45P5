import argparse
import sys
import os
from typing import Union



def bin2header(
    input:Union[str,bytes], # pylint: disable=redefined-builtin
    output_path:str=None, 
    var_name:str='DATA',
    attributes:str=None,
) -> str:
    """Generate C header file from binary input file

    Arguments:
        input: Either path to binary file or binary contents of previously loaded file
        output_path: Output file. Use input with .h appended if omitted
        var_name: Name of C array. Use input filename if omitted
        attributes: Attributes to prepend C array variable

    Returns:
        Path to generated C header
    """
    def _get_ascii(c):
        if c >= 32 and c < 127 and chr(c) not in '*#\\':
            return chr(c) 
        return '.'

    if attributes is None:
        attributes = ''

    if isinstance(input, str):
        if output_path is None:
            output_path = input + '.h'
        
        with open(input, 'rb') as f:
            data = f.read()
    else:
        if output_path is None:
            raise Exception('Must provide output_path if input argument is not a file path')
        
        data = input

    out =''
    out += f'const unsigned char {var_name}[{len(data)}] {attributes} =\n{{\n'
   
    l = [ data[i:i+16] for i in range(0, len(data), 16) ]
    max_line_len = 0
    for i, x in enumerate(l):
        line = ','.join([ '0x{val:02X}'.format(val=c) for c in x ])
        if len(line) > max_line_len:
            max_line_len = len(line) + 1
        out += line 
        if i < len(l) -1:
            out += ','
        else:
            out += ' ' * (max_line_len - len(line))

        out += f' /* {i*16:6d}: '
        out += ''.join([_get_ascii(c) for c in x])
        out += ' */'
        out += '\n'

    out += '};\n'

    if output_path:
        with open(output_path, 'w') as f:
            f.write(out)
    
    return out    



def main():
    parser = argparse.ArgumentParser(description='Generate C header file from binary input file')
    parser.add_argument('input', help='Input file')
    parser.add_argument('-o', '--output', required=False , help='Output file. Use input with .h appended if omitted')
    parser.add_argument('-n', '--name', required=False , help='Name of C array. Use input filename if omitted')
    parser.add_argument('-a', '--attributes', default=None, help='Attributes to prepend C array variable')

    args = parser.parse_args()
    if not args:
        return 1


    if not args.name:
        args.name = os.path.splitext(os.path.basename(args.input))[0].upper().replace('-', '_').replace(' ', '_').replace('.', '_')

    bin2header(
        input=args.input, 
        output_path=args.output, 
        var_name=args.name,
        attributes=args.attributes,
    )

    return 0


if __name__ == '__main__':
    sys.exit(main())