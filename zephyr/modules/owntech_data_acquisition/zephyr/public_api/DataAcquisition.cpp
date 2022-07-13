/*
 * Copyright (c) 2022 LAAS-CNRS
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGLPV2.1
 */

/**
 * @date   2022
 *
 * @author Clément Foucher <clement.foucher@laas.fr>
 * @author Luiz Villa <luiz.villa@laas.fr>
 */


// Stdlib
#include <string.h>

// Current module private functions
#include "../dma/dma.h"
#include "../data_dispatch/data_dispatch.h"
#include "../data_conversion/data_conversion.h"

// OwnTech Power API
#include "adc.h"

// Current class header
#include "DataAcquisition.h"


/////
// Public object to interact with the class

DataAcquisition dataAcquisition;


/////
// Initialize static members

DataAcquisition::channel_assignment_t DataAcquisition::v1_low_assignement      = {0};
DataAcquisition::channel_assignment_t DataAcquisition::v2_low_assignement      = {0};
DataAcquisition::channel_assignment_t DataAcquisition::v_high_assignement      = {0};
DataAcquisition::channel_assignment_t DataAcquisition::i1_low_assignement      = {0};
DataAcquisition::channel_assignment_t DataAcquisition::i2_low_assignement      = {0};
DataAcquisition::channel_assignment_t DataAcquisition::i_high_assignement      = {0};
DataAcquisition::channel_assignment_t DataAcquisition::temp_sensor_assignement = {0};


/////
// Public static configuration functions

void  DataAcquisition::setChannnelAssignment(uint8_t adc_number, const char* channel_name, uint8_t channel_rank)
{
	if (strcmp(channel_name, "V1_LOW") == 0)
	{
		v1_low_assignement.adc_number   = adc_number;
		v1_low_assignement.channel_rank = channel_rank;
	}
	else if (strcmp(channel_name, "V2_LOW") == 0)
	{
		v2_low_assignement.adc_number   = adc_number;
		v2_low_assignement.channel_rank = channel_rank;
	}
	else if (strcmp(channel_name, "V_HIGH") == 0)
	{
		v_high_assignement.adc_number   = adc_number;
		v_high_assignement.channel_rank = channel_rank;
	}
	else if (strcmp(channel_name, "I1_LOW") == 0)
	{
		i1_low_assignement.adc_number   = adc_number;
		i1_low_assignement.channel_rank = channel_rank;
	}
	else if (strcmp(channel_name, "I2_LOW") == 0)
	{
		i2_low_assignement.adc_number   = adc_number;
		i2_low_assignement.channel_rank = channel_rank;
	}
	else if (strcmp(channel_name, "I_HIGH") == 0)
	{
		i_high_assignement.adc_number   = adc_number;
		i_high_assignement.channel_rank = channel_rank;
	}
	else if (strcmp(channel_name, "TEMP_SENSOR") == 0)
	{
		temp_sensor_assignement.adc_number   = adc_number;
		temp_sensor_assignement.channel_rank = channel_rank;
	}
}

void DataAcquisition::start()
{
	uint8_t number_of_adcs = 2;

	for (uint8_t adc_num = 1 ; adc_num <= number_of_adcs ; adc_num++)
	{
		uint8_t channel_rank = 0;

		while (1)
		{
			const char* channel_name = adc_get_channel_name(adc_num, channel_rank);

			if (channel_name != NULL)
			{
				setChannnelAssignment(adc_num, channel_name, channel_rank);
				channel_rank++;
			}
			else
			{
				break;
			}
		}

	}

	// DMAs
	dma_configure_and_start(number_of_adcs);

	// Initialize data dispatch
	data_dispatch_init(number_of_adcs);

	// Launch ADC conversion
	adc_start();
}


/////
// Public static accessors

uint16_t* DataAcquisition::getV1LowRawValues(uint32_t& number_of_values_acquired)
{
	return data_dispatch_get_acquired_values(v1_low_assignement.adc_number, v1_low_assignement.channel_rank, &number_of_values_acquired);
}

uint16_t* DataAcquisition::getV2LowRawValues(uint32_t& number_of_values_acquired)
{
	return data_dispatch_get_acquired_values(v2_low_assignement.adc_number, v2_low_assignement.channel_rank, &number_of_values_acquired);
}

uint16_t* DataAcquisition::getVHighRawValues(uint32_t& number_of_values_acquired)
{
	return data_dispatch_get_acquired_values(v_high_assignement.adc_number, v_high_assignement.channel_rank, &number_of_values_acquired);
}

uint16_t* DataAcquisition::getI1LowRawValues(uint32_t& number_of_values_acquired)
{
	return data_dispatch_get_acquired_values(i1_low_assignement.adc_number, i1_low_assignement.channel_rank, &number_of_values_acquired);
}

uint16_t* DataAcquisition::getI2LowRawValues(uint32_t& number_of_values_acquired)
{
	return data_dispatch_get_acquired_values(i2_low_assignement.adc_number, i2_low_assignement.channel_rank, &number_of_values_acquired);
}

uint16_t* DataAcquisition::getIHighRawValues(uint32_t& number_of_values_acquired)
{
	return data_dispatch_get_acquired_values(i_high_assignement.adc_number, i_high_assignement.channel_rank, &number_of_values_acquired);
}

uint16_t* DataAcquisition::getTemperatureRawValues(uint32_t& number_of_values_acquired)
{
	return data_dispatch_get_acquired_values(temp_sensor_assignement.adc_number, temp_sensor_assignement.channel_rank, &number_of_values_acquired);
}

float32_t DataAcquisition::getV1Low()
{
	uint32_t data_count;
	static float32_t converted_data = -10000; // Return an impossible value if no data has been acquired yet
	uint16_t* buffer = getV1LowRawValues(data_count);

	if (data_count > 0) // If data was received it gets converted
	{
		uint16_t raw_value = buffer[data_count - 1];
		converted_data = data_conversion_convert_v1_low(raw_value);
	}

	return converted_data;
}

float32_t DataAcquisition::getV2Low()
{
	uint32_t data_count;
	static float32_t converted_data = -10000; // Return an impossible value if no data has been acquired yet
	uint16_t* buffer = getV2LowRawValues(data_count);

	if (data_count > 0) // If data was received it gets converted
	{
		uint16_t raw_value = buffer[data_count - 1];
		converted_data = data_conversion_convert_v2_low(raw_value);
	}

	return converted_data;
}

float32_t DataAcquisition::getVHigh()
{
	uint32_t data_count;
	static float32_t converted_data = -10000; // Return an impossible value if no data has been acquired yet
	uint16_t* buffer = getVHighRawValues(data_count);

	if (data_count > 0) // If data was received it gets converted
	{
		uint16_t raw_value = buffer[data_count - 1];
		converted_data = data_conversion_convert_v_high(raw_value);
	}

	return converted_data;
}

float32_t DataAcquisition::getI1Low()
{
	uint32_t data_count;
	static float32_t converted_data = -10000; // Return an impossible value if no data has been acquired yet
	uint16_t* buffer = getI1LowRawValues(data_count);

	if (data_count > 0) // If data was received it gets converted
	{
		uint16_t raw_value = buffer[data_count - 1];
		converted_data = data_conversion_convert_i1_low(raw_value);
	}

	return converted_data;
}

float32_t DataAcquisition::getI2Low()
{
	uint32_t data_count;
	static float32_t converted_data = -10000; // Return an impossible value if no data has been acquired yet
	uint16_t* buffer = getI2LowRawValues(data_count);

	if (data_count > 0) // If data was received it gets converted
	{
		uint16_t raw_value = buffer[data_count - 1];
		converted_data = data_conversion_convert_i2_low(raw_value);
	}

	return converted_data;
}

float32_t DataAcquisition::getIHigh()
{
	uint32_t data_count;
	static float32_t converted_data = -10000; // Return an impossible value if no data has been acquired yet
	uint16_t* buffer = getIHighRawValues(data_count);

	if (data_count > 0) // If data was received it gets converted
	{
		uint16_t raw_value = buffer[data_count - 1];
		converted_data = data_conversion_convert_i_high(raw_value);
	}

	return converted_data;
}

float32_t DataAcquisition::getTemperature()
{
	uint32_t data_count;
	static float32_t converted_data = -10000; // Return an impossible value if no data has been acquired yet
	uint16_t* buffer = getTemperatureRawValues(data_count);

	if (data_count > 0) // If data was received it gets converted
	{
		uint16_t raw_value = buffer[data_count - 1];
		converted_data = data_conversion_convert_temp(raw_value);
	}

	return converted_data;
}

float32_t DataAcquisition::convertV1Low(uint16_t raw_value)
{
	return data_conversion_convert_v1_low(raw_value);
}

float32_t DataAcquisition::convertV2Low(uint16_t raw_value)
{
	return data_conversion_convert_v2_low(raw_value);
}

float32_t DataAcquisition::convertVHigh(uint16_t raw_value)
{
	return data_conversion_convert_v_high(raw_value);
}

float32_t DataAcquisition::convertI1Low(uint16_t raw_value)
{
	return data_conversion_convert_i1_low(raw_value);
}

float32_t DataAcquisition::convertI2Low(uint16_t raw_value)
{
	return data_conversion_convert_i2_low(raw_value);
}

float32_t DataAcquisition::convertIHigh(uint16_t raw_value)
{
	return data_conversion_convert_i_high(raw_value);
}

float32_t DataAcquisition::convertTemperature(uint16_t raw_value)
{
	return data_conversion_convert_temp(raw_value);
}


void DataAcquisition::setV1LowParameters(float32_t gain, float32_t offset)
{
	data_conversion_set_v1_low_parameters(gain, offset);
}

void DataAcquisition::setV2LowParameters(float32_t gain, float32_t offset)
{
	data_conversion_set_v2_low_parameters(gain, offset);
}

void DataAcquisition::setVHighParameters(float32_t gain, float32_t offset)
{
	data_conversion_set_v_high_parameters(gain, offset);
}

void DataAcquisition::setI1LowParameters(float32_t gain, float32_t offset)
{
	data_conversion_set_i1_low_parameters(gain, offset);
}

void DataAcquisition::setI2LowParameters(float32_t gain, float32_t offset)
{
	data_conversion_set_i2_low_parameters(gain, offset);
}

void DataAcquisition::setIHighParameters(float32_t gain, float32_t offset)
{
	data_conversion_set_i_high_parameters(gain, offset);
}

void DataAcquisition::setTemperatureParameters(float32_t gain, float32_t offset)
{
	data_conversion_set_temp_parameters(gain, offset);
}