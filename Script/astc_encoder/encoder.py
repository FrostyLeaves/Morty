
import os
import sys
from pathlib import Path
from wand.image import Image

current_file_path = Path(__file__).resolve()


astc_encoder_execute = "astcenc-sse4.1.exe"
astc_encoder_execute_fullpath = str(current_file_path.parent) + "/" + astc_encoder_execute

def astc_encoder(full_path: Path):
    astc_path = full_path.with_suffix(".astc")
    command = astc_encoder_execute_fullpath + " -cl " + str(full_path) + " " + str(astc_path) + " 4x4 -thorough"

    print("execute command: "+ command)
    os.system(command)

def dds_encoder(full_path: Path):
    dds_path = full_path.with_suffix(".dds")
    image = Image(filename = str(full_path))
    image.compression = "dxt5"
    image.save(filename=dds_path)



def iterator_path(path, encoder_func):
    for file_path, _, file_names in os.walk(path):
        for file_name in file_names:
            full_path = Path(os.path.join(file_path, file_name))
            if full_path.suffix == ".png":
                print("execute command: "+ str(full_path))
                encoder_func(full_path)
            


if __name__ == "__main__":
    path = sys.argv[1]
    iterator_path(path, dds_encoder)
    input('finished.')