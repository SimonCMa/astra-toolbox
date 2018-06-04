/*
-----------------------------------------------------------------------
Copyright: 2010-2016, iMinds-Vision Lab, University of Antwerp
           2014-2016, CWI, Amsterdam

Contact: astra@uantwerpen.be
Website: http://www.astra-toolbox.com/

This file is part of the ASTRA Toolbox.


The ASTRA Toolbox is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The ASTRA Toolbox is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the ASTRA Toolbox. If not, see <http://www.gnu.org/licenses/>.

-----------------------------------------------------------------------
*/

#include "astra/CudaProjector3D.h"

#include "astra/VolumeGeometry3D.h"
#include "astra/ProjectionGeometry3D.h"

#include "astra/ConeProjectionGeometry3D.h"
#include "astra/ConeVecProjectionGeometry3D.h"


namespace astra
{

// type of the projector, needed to register with CProjectorFactory
std::string CCudaProjector3D::type = "cuda3d";


//----------------------------------------------------------------------------------------
// Default constructor
CCudaProjector3D::CCudaProjector3D()
{
	_clear();
}

//----------------------------------------------------------------------------------------
// Destructor
CCudaProjector3D::~CCudaProjector3D()
{
	if (m_bIsInitialized) clear();
}

//----------------------------------------------------------------------------------------
// Clear for constructors
void CCudaProjector3D::_clear()
{
	m_pProjectionGeometry = NULL;
	m_pVolumeGeometry = NULL;
	m_bIsInitialized = false;

	m_projectionKernel = ker3d_default;
	m_iVoxelSuperSampling = 1;
	m_iDetectorSuperSampling = 1;
	m_iGPUIndex = -1;
	m_bDensityWeighting = false;
}

//----------------------------------------------------------------------------------------
// Clear
void CCudaProjector3D::clear()
{
	ASTRA_DELETE(m_pProjectionGeometry);
	ASTRA_DELETE(m_pVolumeGeometry);
	m_bIsInitialized = false;
}

//----------------------------------------------------------------------------------------
// Check
bool CCudaProjector3D::_check()
{
	// projection geometry
	ASTRA_CONFIG_CHECK(m_pProjectionGeometry, "CudaProjector3D", "ProjectionGeometry3D not initialized.");
	ASTRA_CONFIG_CHECK(m_pProjectionGeometry->isInitialized(), "CudaProjector3D", "ProjectionGeometry3D not initialized.");

	// volume geometry
	ASTRA_CONFIG_CHECK(m_pVolumeGeometry, "CudaProjector3D", "VolumeGeometry3D not initialized.");
	ASTRA_CONFIG_CHECK(m_pVolumeGeometry->isInitialized(), "CudaProjector3D", "VolumeGeometry3D not initialized.");

	return true;
}


//---------------------------------------------------------------------------------------
// Initialize, use a Config object
bool CCudaProjector3D::initialize(const Config& _cfg)
{
	// if already initialized, clear first
	if (m_bIsInitialized) {
		clear();
	}

	// initialization of parent class
	if (!CProjector3D::initialize(_cfg)) {
		return false;
	}

        if(!initialize_parameters(_cfg)) {
                return false;
        }

        m_bIsInitialized = _check();
	return m_bIsInitialized;
}


// SCM: need additional function which initializes this object only
// as a "parameter-struct" as used in CompositeGeometryManager.
bool CCudaProjector3D::initialize_parameters(const Config& _cfg) {

	assert(_cfg.self);
	ConfigStackCheck<CProjector3D> CC("CudaProjector3D", this, _cfg);

        XMLNode node = _cfg.self.getSingleNode("ProjectionKernel");
	m_projectionKernel = ker3d_default;
	if (node) {
		std::string sProjKernel = node.getContent();

		if (sProjKernel == "default") {

		} else if (sProjKernel == "sum_square_weights") {
			m_projectionKernel = ker3d_sum_square_weights;
		} else if (sProjKernel == "bicubic") {
			m_projectionKernel = ker3d_bicubic;
		} else if (sProjKernel == "bicubic_derivative_1") {
			m_projectionKernel = ker3d_bicubic_ddf1;
		} else if (sProjKernel == "bicubic_derivative_2") {
			m_projectionKernel = ker3d_bicubic_ddf2;
		} else if (sProjKernel == "bspline3") {
			m_projectionKernel = ker3d_bspline3;
		} else if (sProjKernel == "bspline3_derivative_1") {
			m_projectionKernel = ker3d_bspline3_ddf1;
		} else if (sProjKernel == "bspline3_derivative_2") {
			m_projectionKernel = ker3d_bspline3_ddf2;
		} else {
                        ASTRA_CONFIG_CHECK(false, "CudaProjector3D",
                        "Invalid projection-kernel assigned. Admissible choices are \"default\", \"sum_square_weights\", \"bicubic\", \"bicubic_derivative_1\", \"bicubic_derivative_2\", \"bspline3\", \"bspline3_derivative_1\", \"bspline3_derivative_2\".");
			return false;
		}
	}
	CC.markNodeParsed("ProjectionKernel");

	m_iVoxelSuperSampling = (int)_cfg.self.getOptionNumerical("VoxelSuperSampling", 1);
	CC.markOptionParsed("VoxelSuperSampling");

	m_iDetectorSuperSampling = (int)_cfg.self.getOptionNumerical("DetectorSuperSampling", 1);
	CC.markOptionParsed("DetectorSuperSampling");

	if (m_pProjectionGeometry == NULL || 
            dynamic_cast<CConeProjectionGeometry3D*>(m_pProjectionGeometry) ||
	    dynamic_cast<CConeVecProjectionGeometry3D*>(m_pProjectionGeometry))
	{
		m_bDensityWeighting = _cfg.self.getOptionBool("DensityWeighting", false);
		CC.markOptionParsed("DensityWeighting");
	}

	m_iGPUIndex = (int)_cfg.self.getOptionNumerical("GPUindex", -1);
	m_iGPUIndex = (int)_cfg.self.getOptionNumerical("GPUIndex", m_iGPUIndex);
	CC.markOptionParsed("GPUIndex");
	if (!_cfg.self.hasOption("GPUIndex"))
		CC.markOptionParsed("GPUindex");

	return true;

}


/*
bool CProjector3D::initialize(astra::CProjectionGeometry3D *, astra::CVolumeGeometry3D *)
{
	ASTRA_ASSERT(false);

	return false;
}
*/

std::string CCudaProjector3D::description() const
{
	return "";
}

} // end namespace
