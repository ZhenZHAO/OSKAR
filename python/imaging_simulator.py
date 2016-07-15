# 
#  This file is part of OSKAR.
# 
# Copyright (c) 2016, The University of Oxford
# All rights reserved.
#
#  This file is part of the OSKAR package.
#  Contact: oskar at oerc.ox.ac.uk
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#  1. Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#  3. Neither the name of the University of Oxford nor the names of its
#     contributors may be used to endorse or promote products derived from this
#     software without specific prior written permission.
# 
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
# 

from simulator import Simulator


class ImagingSimulator(Simulator):
    """Simulates and images visibilities concurrently.

    Inherits oskar.Simulator to image each block in the process_block() method.
    """

    def __init__(self, imagers, precision='double'):
        """Creates the simulator, storing a handle to the imagers.

        Args:
            imagers (oskar.Imager list): List of OSKAR imagers to use.
            precision (str): Either 'double' or 'single' to specify
                the numerical precision of the simulation.
        """
        Simulator.__init__(self, precision)
        self._imagers = imagers


    def finalise(self):
        """Called automatically by the base class at the end of run()."""
        Simulator.finalise(self)
        for im in self._imagers:
            im.finalise()


    def process_block(self, block, block_index):
        """Writes the visibility block to any open file(s), and images it.

        Args:
            block (oskar.VisBlock): A handle to the block to be processed.
            block_index (int):      The index of the visibility block.
        """
        if not self.coords_only:
            self.write_block(block, block_index)
        for im in self._imagers:
            im.update_from_block(self.vis_header(), block)


    def run(self):
        """Runs the simulator and imagers."""
        # Iterate imagers to find any with uniform weighting or W-projection.
        need_coords_first = False
        for im in self._imagers:
            if im.weighting == 'Uniform' or im.algorithm == 'W-projection':
                need_coords_first = True

        # Simulate coordinates first, if required.
        if need_coords_first:
            self.set_coords_only(True)
            Simulator.run(self, process_blocks=True)
            self.set_coords_only(False)

        # Simulate and image the visibilities.
        Simulator.run(self, process_blocks=True)


    def set_coords_only(self, value):
        """Calls set_coords_only() on simulator and imager objects."""
        Simulator.set_coords_only(self, value)
        for im in self._imagers:
            im.set_coords_only(value)
