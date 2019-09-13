
#include <gtest/gtest.h>
#include "Message.h"
#include "Channel.h"
#include "writers/GadgetIsmrmrdWriter.h"
#include "readers/GadgetIsmrmrdReader.h"
#include <sstream>
#include "hoNDArray_elemwise.h"
#include "mri_core_data.h"
#include "readers/BufferReader.h"
#include "readers/IsmrmrdImageArrayReader.h"
#include "writers/BufferWriter.h"
#include "writers/IsmrmrdImageArrayWriter.h"
#include "MessageID.h"

TEST(ReadWriteTest,AcquisitionTest){
    using namespace Gadgetron;
	using namespace Gadgetron::Core;

    auto stream = std::stringstream{};

    auto acquisition_header = ISMRMRD::AcquisitionHeader();
    acquisition_header.number_of_samples = 32;
    acquisition_header.active_channels = 1;
    acquisition_header.available_channels = 1;

    auto data= hoNDArray<std::complex<float>>(32,1);
    data.fill(42.0f);


    auto message = Core::Message(acquisition_header,data);


    auto reader = GadgetIsmrmrdAcquisitionMessageReader();
    auto writer = GadgetIsmrmrdAcquisitionMessageWriter();


    ASSERT_TRUE(writer.accepts(message));

    writer.write(stream,std::move(message));

	ASSERT_EQ(Core::IO::read<uint16_t>(stream), GADGET_MESSAGE_ISMRMRD_ACQUISITION);

	auto read_message = reader.read(stream);

    auto unpacked = Core::unpack<Core::Acquisition>(std::move(read_message));

    EXPECT_TRUE(bool(unpacked));

	auto value = *unpacked;
    ASSERT_EQ(data,std::get<hoNDArray<std::complex<float>>>(value));
}


TEST(ReadWriteTest, BufferTest){
	using namespace Gadgetron;
	using namespace Gadgetron::Core;

	IsmrmrdReconData recondata;
	recondata.rbit_.push_back(IsmrmrdReconBit());

	auto& rbit = recondata.rbit_.back();
	rbit.data_.data_ = hoNDArray<std::complex<float>>(42, 42);
	rbit.data_.data_.fill(4242.0f);


	auto stream = std::stringstream();
	auto message = Core::Message(recondata);
	auto reader = Core::Readers::BufferReader();
	auto writer = Core::Writers::BufferWriter();

	ASSERT_TRUE(writer.accepts(message));

	writer.write(stream,std::move( message));

	ASSERT_EQ(Core::IO::read<uint16_t>(stream), GADGET_MESSAGE_RECONDATA);

	auto unpacked = Core::unpack<IsmrmrdReconData>(reader.read(stream));

	EXPECT_TRUE(bool(unpacked));

	auto value = *unpacked;

	ASSERT_EQ(rbit.data_.data_, value.rbit_.back().data_.data_);

}


TEST(ReadWriteTest, ImageArrayTest) {
	using namespace Gadgetron;
	using namespace Gadgetron::Core;

	IsmrmrdImageArray img_array;
	img_array.data_ = hoNDArray<std::complex<float>>(42, 42);


	auto stream = std::stringstream();
	auto message = Core::Message(img_array);
	auto reader = Core::Readers::IsmrmrdImageArrayReader();
	auto writer = Core::Writers::IsmrmrdImageArrayWriter();

	ASSERT_TRUE(writer.accepts(message));

	writer.write(stream, std::move(message));

	ASSERT_EQ(Core::IO::read<uint16_t>(stream), GADGET_MESSAGE_ISMRMRD_IMAGE_ARRAY);

	auto unpacked = Core::unpack<IsmrmrdImageArray>(reader.read(stream));

	EXPECT_TRUE(bool(unpacked));

	auto value = *unpacked;

	ASSERT_EQ(img_array.data_, value.data_);

}