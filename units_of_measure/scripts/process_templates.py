#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import time
import json
import errno
import jinja2
import pathlib
import argparse

from typing import List
from jinja2 import Environment, FileSystemLoader, select_autoescape
from datetime import datetime

SI_PREFIX = {
    "femto": 1e-18,
    "pico": 1e-12,
    "nano": 1e-9,
    "micro": 1e-6,
    "milli": 1e-3,
    "deci": 1e-2,
    "iso": 1,
    "deca": 1e2,
    "kilo": 1e3,
    "mega": 1e6,
    "giga": 1e9,
    "terra": 1e9,
    "peta": 1e12,
    "zetta": 1e15,
}

IEC_PREFIX = {
    "iso": 2**0,
    "kibi": 2**10,
    "mebi": 2**20,
    "gibi": 2**30,
}

SCALE_TYPES = {
    "SI::prefix": SI_PREFIX,
    "IEC::prefix": IEC_PREFIX,
}


def preprocess_json(json_file: str, outdir: str) -> int:
    if not os.path.exists(json_file):
        return -ENOENT
    if not os.path.exists(outdir):
        os.makedirs(outdir)
    with open(json_file, "r") as file_data:
        json_data = json.load(file_data)
        for unit in json_data["units"]:
            output_filepath = os.path.join(
                outdir, "source", unit["namespace"], f"{unit['classname']}.cpp"
            )
            comment = unit["comment"] if "comment" in unit else ""
            preprocess_template(
                namespace=unit["namespace"],
                classname=unit["classname"],
                typename=unit["typename"],
                scale=unit["scalevalue"],
                scaletype=unit["scaletype"],
                postfix=unit["postfix"],
                comment=comment,
                input_filepath=json_data["source_template"],
                output_filepath=output_filepath,
            )
            output_filepath = os.path.join(
                outdir, "include", unit["namespace"], f"{unit['classname']}.hpp"
            )
            preprocess_template(
                namespace=unit["namespace"],
                classname=unit["classname"],
                typename=unit["typename"],
                scale=unit["scalevalue"],
                scaletype=unit["scaletype"],
                postfix=unit["postfix"],
                comment=comment,
                input_filepath=json_data["header_template"],
                output_filepath=output_filepath,
            )


def preprocess_template(
    namespace: str,
    classname: str,
    typename: str,
    scale: str,
    scaletype: str,
    postfix: str,
    input_filepath: str,
    output_filepath: str,
    comment: str,
) -> bool:
    input_path = pathlib.Path(input_filepath).parent.absolute()
    print(
        f'[Preprocessing] {classname} as {typename} at scale {scaletype}::{scale} and postfix "{postfix}"'
    )

    environment = jinja2.Environment(
        loader=FileSystemLoader(input_path),
        autoescape=select_autoescape(["cpp", "hpp"]),
    )
    template = environment.get_template(pathlib.Path(input_filepath).name)
    # probably could change this to use kwargs
    variables = {
        "namespace": namespace,
        "classname": classname,
        "typename": typename,
        "scale": scale,
        "scaletype": scaletype,
        "postfix": postfix,
        "comment": comment,
        "year": datetime.now().year,
    }
    output = template.render(**variables)
    directory = pathlib.Path(output_filepath).parent
    print("Check for directory? {}".format(directory))
    if not os.path.exists(directory):
        os.makedirs(directory)
    with open(output_filepath, "w") as filedata:
        filedata.write(output)
        print(f"Wrote to file {output_filepath}")

    return 0


def main(args: List[str]) -> int:
    parser = argparse.ArgumentParser(
        description="A script to generate Units of Measure",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    group1 = parser.add_argument_group("Breakout")
    group2 = parser.add_argument_group("Collection")
    group1.add_argument(
        "-n",
        "--namespace",
        type=str,
        default=None,
        help="The C++ Namespace in which to put the class",
    )
    group1.add_argument(
        "-c",
        "--classname",
        type=str,
        default=None,
        help="The Name of the Class to Create",
    )
    group1.add_argument(
        "-t",
        "--typename",
        type=str,
        default="float",
        help="The fundamental type which will contain value",
    )
    group1.add_argument(
        "-s",
        "--scale",
        type=str,
        choices=SI_PREFIX.keys(),
        default="iso",
        help="The scale of the unit",
    )
    group1.add_argument(
        "-st",
        "--scaletype",
        type=str,
        choices=SCALE_TYPES.keys(),
        default="SI::prefix",
        help="The scaling factors of this unit",
    )
    group1.add_argument(
        "-p",
        "--postfix",
        type=str,
        default=None,
        help="The quote operator literal constructor",
    )
    group1.add_argument(
        "-if",
        "--infile",
        type=str,
        default=None,
        help="The name of the input jinja template file",
    )
    group1.add_argument(
        "-of",
        "--outfile",
        type=str,
        default=None,
        help="The name of the output file.",
    )
    group2.add_argument(
        "-j",
        "--json",
        type=str,
        default=None,
        help="JSON File Containing all types together.",
    )
    group2.add_argument(
        "-od", "--outdir", type=str, default=None, help="The output files directory"
    )
    parsed = parser.parse_args(args)
    if parsed.json is not None:
        return preprocess_json(json_file=parsed.json, outdir=parsed.outdir)
    else:
        return preprocess_template(
            namespace=parsed.namespace,
            classname=parsed.classname,
            typename=parsed.typename,
            scale=parsed.scale,
            scaletype=parsed.scaletype,
            postfix=parsed.postfix,
            input_filepath=parsed.infile,
            output_filepath=parsed.outfile,
            comment=parsed.comment
        )


if __name__ == "__main__":
    sys.exit(0 if main(sys.argv[1:]) else -1)
