function astra_set_gpu_index(index)

%--------------------------------------------------------------------------
% Set the index of the GPU to use
%--------------------------------------------------------------------------
%--------------------------------------------------------------------------
% This file is part of the ASTRA Toolbox
% 
% Copyright: 2010-2016, iMinds-Vision Lab, University of Antwerp
%            2014-2016, CWI, Amsterdam
% License: Open Source under GPLv3
% Contact: astra@uantwerpen.be
% Website: http://www.astra-toolbox.com/
%--------------------------------------------------------------------------

if nargin < 1
    astra_mex('get_gpu_info');
else
    astra_mex('get_gpu_info', index);
end
