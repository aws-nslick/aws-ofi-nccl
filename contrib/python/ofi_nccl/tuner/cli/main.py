import typer
import pathlib
import pandas as pd
import os
from .wrapper import NCCLFunc, NCCLProto, NCCLAlgo, Tuner
app = typer.Typer(name="show-tuner-decisions")

@app.command()
def show(library: pathlib.Path):
    ranks_per_nodes = [ 1, 2, 4, 8 ]
    nnodes = [ 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048 ]
    os.environ['OFI_NCCL_FORCE_PRODUCT_NAME'] = "p5en.48xlarge"
    for rpn in ranks_per_nodes:
        for nodecnt in nnodes:
            t = Tuner(library, nranks=(rpn*nodecnt), nnodes=nodecnt)
            print(t.analyze_all())
