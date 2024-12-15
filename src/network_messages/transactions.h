#pragma once

#include <scale.h>
#include "common_def.h"

// used with Transaction
struct ContractIPOBid
{
    long long price;
    unsigned short quantity;
};

#define BROADCAST_TRANSACTION 24


// A transaction is made of this struct, followed by inputSize Bytes payload data and SIGNATURE_SIZE Bytes signature
struct Transaction
{
    m256i sourcePublicKey;
    m256i destinationPublicKey;
    long long amount;
    unsigned int tick;
    unsigned short inputType;
    unsigned short inputSize;

    void print() {
      std::cout << "Transaction:\n\t" << "SourcePublicKey: " <<
                                      (int32_t)*(this->sourcePublicKey.m256i_i32) <<
                                      "\n\tDestPublicKey: " <<
                                        (int32_t)*(this->destinationPublicKey.m256i_i32) <<
                                        "\n\tAmount: " << (this->amount) <<
                                        "\n\tTick: " << (this->tick) <<
                                        "\n\tinputType: " << (this->inputType) <<
                                        "\n\tinputSize: " << (this->inputSize) << std::endl;
    }


    /* read a void* Structure @structure and write a serialized scale byte array of length @bytes to @serialized */
    void Transaction_serialize(uint8_t* serialized, size_t *bytes, void *structure) {
      struct Transaction *value = (struct Transaction*)structure; //Cast void* as our Structure
      scale_fixed_int amount;
      scale_fixed_int tick;
      scale_fixed_int inputType;
      scale_fixed_int inputSize;

      int8_t *sourcePubKeyVec = value->sourcePublicKey.m256i_i8;
      int8_t *destPubKeyVec = value->destinationPublicKey.m256i_i8;
      scale_vector vec1 =  {.data=NULL, .data_len=0, .prefix_num_elements={.mode=(scale_compact_int_mode)0, .mode_upper_bits=0, .data=NULL}};
      scale_vector vec2 =  {.data=NULL, .data_len=0, .prefix_num_elements={.mode=(scale_compact_int_mode)0, .mode_upper_bits=0, .data=NULL}};
      int i;
      for(i=0; i <32; i++) {
        int8_t temp = sourcePubKeyVec[i];
        scale_fixed_int fixed = { 0 };
        encode_int_to_fixed_int_scale(&fixed, temp);
        uint8_t serialized[2] = { 0 };
        uint64_t serialized_len = 0;
        serialize_fixed_int(serialized, &serialized_len, &fixed);
        push_vector(&vec1, serialized, serialized_len);
      }

      for(i=0; i <32; i++) {
        int8_t temp = destPubKeyVec[i];
        scale_fixed_int fixed = { 0 };
        encode_int_to_fixed_int_scale(&fixed, temp);
        uint8_t serialized[2] = { 0 };
        uint64_t serialized_len = 0;
        serialize_fixed_int(serialized, &serialized_len, &fixed);
        push_vector(&vec2, serialized, serialized_len);
      }

      encode_int_to_fixed_int_scale(&amount, value->amount); //encode Structure.b as a fixed int
      encode_int_to_fixed_int_scale(&tick, value->tick); //encode Structure.b as a fixed int
      encode_int_to_fixed_int_scale(&inputType, value->inputType);
      encode_int_to_fixed_int_scale(&inputSize, value->inputSize);

      uint64_t len = 0;

      serialize_vector(&serialized[0], (size_t*)&len, &vec1);
      *bytes += len;

      serialize_vector(&serialized[len - 1], (size_t*)&len, &vec2);
      *bytes += len;

      serialize_fixed_int(&serialized[*bytes - 1], &len, &amount); //serialize the fixed int and write it to out bytes
      *bytes += len;
      serialize_fixed_int(&serialized[*bytes - 1], &len, &tick); //serialize the fixed int and write it to out bytes
      *bytes += len;
      serialize_fixed_int(&serialized[*bytes - 1], &len, &inputType); //serialize the fixed int and write it to out bytes
      *bytes += len;
      serialize_fixed_int(&serialized[*bytes - 1], &len, &inputSize); //serialize the fixed int and write it to out bytes
      *bytes += len;
    }

    void Transaction_deserialize(void *structure_out, uint8_t *bytes, size_t len) {
      struct Transaction *value = (struct Transaction*)structure_out; //Cast this void* to our Structure

      int8_t srcPubKey[32] = { 0 };
      int8_t dstPubKey[32] = { 0 };

      scale_vector vector = {.data=NULL, .data_len=0, .prefix_num_elements={.mode=(scale_compact_int_mode)0, .mode_upper_bits=0, .data=NULL}};
      scale_vector vector2 = {.data=NULL, .data_len=0, .prefix_num_elements={.mode=(scale_compact_int_mode)0, .mode_upper_bits=0, .data=NULL}};

      uint8_t width = sizeof(uint8_t); //byte width of each element, perhaps a string (Vec<char>), Vec<i8>, or Vec<Bool>
      size_t bytes_read = read_vector_from_data(&vector, width, bytes);
      read_vector_from_data(&vector2, width, &bytes[bytes_read-1]);
      uint8_t *elem;
      int offset = 0;
      scale_vector_foreach(&elem, sizeof(int8_t), &vector) { //2 byte elements, Vec<fixed_u16>
        int8_t output = 0;
        deserialize_fixed_int((void*)&output, elem, sizeof(int8_t), false);
        srcPubKey[offset] = (int8_t)output;
        offset++;
      }
      cleanup_vector(&vector);
      offset = 0;
      scale_vector_foreach(&elem, sizeof(int8_t), &vector2) { //2 byte elements, Vec<fixed_u16>
        int8_t output = 0;
        deserialize_fixed_int((void*)&output, elem, sizeof(int8_t), false);
        dstPubKey[offset++] = (int8_t)output;
      }
      cleanup_vector(&vector2);
      memcpy(value->sourcePublicKey.m256i_i8, srcPubKey, 32);
      memcpy(value->destinationPublicKey.m256i_i8, dstPubKey, 32);
      int byteOffset = 33 + 33;
      deserialize_fixed_int((int64_t*)&value->amount, &bytes[byteOffset - 1], 8, true);
      byteOffset += 8;
      deserialize_fixed_int((int32_t*)&value->tick, &bytes[byteOffset - 1], 4, true);
      byteOffset += 4;
      deserialize_fixed_int((int16_t*)&value->inputType, &bytes[byteOffset - 1], 2, true);
      byteOffset += 2;
      deserialize_fixed_int((int16_t*)&value->inputSize, &bytes[byteOffset - 1], 2, true);
    }


    // Return total transaction data size with payload data and signature
    unsigned int totalSize() const
    {
        return sizeof(Transaction) + inputSize + SIGNATURE_SIZE;
    }

    // Check if transaction is valid
    bool checkValidity() const
    {
        return amount >= 0 && amount <= MAX_AMOUNT && inputSize <= MAX_INPUT_SIZE;
    }

    // Return pointer to transaction's payload (CAUTION: This is behind the memory reserved for this struct!)
    unsigned char* inputPtr()
    {
        return (((unsigned char*)this) + sizeof(Transaction));
    }

    // Return pointer to transaction's payload (CAUTION: This is behind the memory reserved for this struct!)
    const unsigned char* inputPtr() const
    {
        return (((const unsigned char*)this) + sizeof(Transaction));
    }

    // Return pointer to signature (CAUTION: This is behind the memory reserved for this struct!)
    unsigned char* signaturePtr()
    {
        return ((unsigned char*)this) + sizeof(Transaction) + inputSize;
    }
};

static_assert(sizeof(Transaction) == 32 + 32 + 8 + 4 + 2 + 2, "Something is wrong with the struct size.");

#define REQUEST_TICK_TRANSACTIONS 29

struct RequestedTickTransactions
{
    unsigned int tick;
    unsigned char transactionFlags[NUMBER_OF_TRANSACTIONS_PER_TICK / 8];
};

#define REQUEST_TRANSACTION_INFO 26
struct RequestedTransactionInfo
{
    m256i txDigest;
};

