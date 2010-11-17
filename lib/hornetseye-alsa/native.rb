# hornetseye-alsa - Play audio data using libalsa
# Copyright (C) 2010 Jan Wedekind
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Namespace of Hornetseye computer vision library
module Hornetseye

  class AlsaInput

    attr_reader :rate

    attr_reader :channels

    def close
    end

    def avail
    end

    def delay
    end

    def prepare
    end

  end
  
  class AlsaOutput

    attr_reader :rate

    attr_reader :channels

    def close
    end

    def drop
    end

    def drain
    end

    def avail
    end

    def delay
    end

    def prepare
    end

  end

end

