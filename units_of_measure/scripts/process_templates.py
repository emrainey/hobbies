#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import time
import jinja2
import pathlib
import argparse

from typing import List
from jinja2 import Environment, FileSystemLoader, select_autoescape
from datetime import datetime

SI_PREFIX = {
    'femto': 1E-18,
    'pico': 1E-12,
    'nano': 1E-9,
    'micro': 1E-6,
    'milli': 1E-3,
    'deci': 1E-2,
    'iso': 1,
    'deca': 1E2,
    'kilo': 1E3,
    'mega': 1E6,
    'giga': 1E9,
    'terra': 1E9,
    'peta': 1E12,
    'zetta': 1E15
}

IEC_PREFIX = {
    'iso': 2**0,
    'kibi': 2**10,
    'mebi': 2**20,
    'gibi': 2**30,
}

SCALE_TYPES = {
    'SI::prefix': SI_PREFIX,
    'IEC::prefix': IEC_PREFIX,
}

def preprocess_template(namespace: str, classname: str, typename: str, scale: str, scaletype: str, postfix: str, input_filepath: str, output_filepath: str) -> bool:
    input_path=pathlib.Path(input_filepath).parent.absolute()
    print(f'[Preprocessing] {classname} as {typename} and postfix \"{postfix}\"')

    environment = jinja2.Environment(loader=FileSystemLoader(input_path), autoescape=select_autoescape(['cpp', 'hpp']))
    template = environment.get_template(pathlib.Path(input_filepath).name)
    # probably could change this to use kwargs
    variables = {
        'namespace': namespace,
        'classname': classname,
        'typename': typename,
        'scale': scale,
        'scaletype': scaletype,
        'postfix': postfix,
        'year': datetime.now().year
    }
    output = template.render(**variables)
    directory = pathlib.Path(output_filepath).parent
    if not os.path.exists(directory):
        os.makedirs(directory)
    with open(output_filepath, 'w') as filedata:
        filedata.write(output)
        print(f'Wrote to file {output_filepath}')

    return True

def main(args: List[str]) -> int:
    parser = argparse.ArgumentParser(description='Doozel Arguments', formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('-n','--namespace', type=str, default=None, required=True, help='The C++ Namespace in which to put the class')
    parser.add_argument('-c','--classname', type=str, default=None, required=True, help='The Name of the Class to Create')
    parser.add_argument('-t','--typename', type=str, default='float', required=True, help='The fundamental type which will contain value')
    parser.add_argument('-s','--scale', type=str, choices=SI_PREFIX.keys(), default='iso', help='The scale of the unit')
    parser.add_argument('-st','--scaletype', type=str, choices=SCALE_TYPES.keys(), default='SI::prefix', help='The scaling factors of this unit')
    parser.add_argument('-p' ,'--postfix', type=str, default=None, required=True, help='The quote operator literal constructor')
    parser.add_argument('-if','--infile', type=str, default=None, required=True, help='The name of the input jinja template file')
    parser.add_argument('-of','--outfile', type=str, default=None, required=True, help='The name of the output file.')
    parsed = parser.parse_args(args)
    return preprocess_template(namespace=parsed.namespace,
                               classname=parsed.classname,
                               typename=parsed.typename,
                               scale=parsed.scale,
                               scaletype=parsed.scaletype,
                               postfix=parsed.postfix,
                               input_filepath=parsed.infile,
                               output_filepath=parsed.outfile)

if __name__ == "__main__":
    sys.exit(0 if main(sys.argv[1:]) else -1)