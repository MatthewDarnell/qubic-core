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

    /* read a void* Structure @structure and write a serialized scale byte array of length @bytes to @serialized */
    void Transaction_serialize(uint8_t* serialized, size_t *bytes, void *structure) {
      auto *value = (struct Transaction*)structure;

      //Declare our Scale Values
      scale_vector sourcePubKeyVector =  SCALE_VECTOR_INIT;
      scale_vector destinationPubKeyVector =  SCALE_VECTOR_INIT;
      scale_fixed_int scale_amount;
      scale_fixed_int scale_tick;
      scale_fixed_int scale_inputType;
      scale_fixed_int scale_inputSize;

      //Convert Each Struct Value To Scale
      int pubKeyLen = 32;
      int8_t *srcPubKey = value->sourcePublicKey.m256i_i8;
      int8_t *destPubKey = value->destinationPublicKey.m256i_i8;

      uint8_t temp_serialized[2] = { 0 };
      for(int i=0; i < pubKeyLen; i++) {
        int8_t temp = srcPubKey[i];
        scale_fixed_int scale_temp_value;
        encode_int_to_fixed_int_scale(&scale_temp_value, temp);
        memset(temp_serialized, 0, 2 * sizeof(uint8_t));
        uint64_t serialized_len = 0;
        serialize_fixed_int(temp_serialized, &serialized_len, &scale_temp_value);
        push_vector(&sourcePubKeyVector, temp_serialized, serialized_len);
      }

      for(int i=0; i < pubKeyLen; i++) {
        int8_t temp = destPubKey[i];
        scale_fixed_int scale_temp_value;
        encode_int_to_fixed_int_scale(&scale_temp_value, temp);
        memset(temp_serialized, 0, 2 * sizeof(uint8_t));
        uint64_t serialized_len = 0;
        serialize_fixed_int(temp_serialized, &serialized_len, &scale_temp_value);
        push_vector(&destinationPubKeyVector, temp_serialized, serialized_len);
      }

      encode_int_to_fixed_int_scale(&scale_amount, value->amount);
      encode_int_to_fixed_int_scale(&scale_tick, value->tick);
      encode_int_to_fixed_int_scale(&scale_inputType, value->inputType);
      encode_int_to_fixed_int_scale(&scale_inputSize, value->inputSize);

      //Serialize Scale Values Into Output Byte Array
      uint64_t len = 0;
      serialize_vector(serialized, (size_t*)&len, &sourcePubKeyVector);
      cleanup_vector(&sourcePubKeyVector);
      *bytes = len;
      serialize_vector(&serialized[*bytes], (size_t*)&len, &destinationPubKeyVector);
      cleanup_vector(&destinationPubKeyVector);
      *bytes += len;

      serialize_fixed_int(&serialized[*bytes], &len, &scale_amount); //serialize the fixed int and write it to out bytes
      *bytes += len;
      serialize_fixed_int(&serialized[*bytes], &len, &scale_tick); //serialize the fixed int and write it to out bytes
      *bytes += len;
      serialize_fixed_int(&serialized[*bytes], &len, &scale_inputType); //serialize the fixed int and write it to out bytes
      *bytes += len;
      serialize_fixed_int(&serialized[*bytes], &len, &scale_inputSize); //serialize the fixed int and write it to out bytes
      *bytes += len;
      //Done
    }

    void Transaction_deserialize(void *structure_out, uint8_t *bytes, size_t len) {
      auto *value = (struct Transaction*)structure_out; //Cast this void* to our Structure
      //Prepare Variables To Receive Each Scale Value
      scale_vector sourcePubKeyVector = SCALE_VECTOR_INIT;
      scale_vector destinationPubKeyVector = SCALE_VECTOR_INIT;

      //Parse Each Scale Value in The Byte Stream
      int pubKeyLen = 32;
      int8_t srcPubKey[64] = { 0 };
      int8_t dstPubKey[64] = { 0 };

      uint8_t *elem;
      int offset = 0;
      int8_t output = 0;

      size_t byteOffset = read_vector_from_data(&sourcePubKeyVector, sizeof(int8_t), bytes);
      scale_vector_foreach(&elem, sizeof(int8_t), &sourcePubKeyVector) { //2 byte elements, Vec<fixed_u16>
        output = 0;
        deserialize_fixed_int((void*)&output, elem, sizeof(int8_t), true);
        srcPubKey[offset] = (int8_t)output;
        offset++;
      }
      cleanup_vector(&sourcePubKeyVector);
      offset = 0;

      byteOffset += read_vector_from_data(&destinationPubKeyVector, sizeof(int8_t), &bytes[byteOffset]);
      scale_vector_foreach(&elem, sizeof(int8_t), &destinationPubKeyVector) { //2 byte elements, Vec<fixed_u16>
        output = 0;
        deserialize_fixed_int((void*)&output, elem, sizeof(int8_t), true);
        dstPubKey[offset++] = (int8_t)output;
      }
      cleanup_vector(&destinationPubKeyVector);

      //Populate Source and Dest Pub Keys
      memset(value->sourcePublicKey.m256i_i8, 0, pubKeyLen * sizeof(int8_t));
      memset(value->destinationPublicKey.m256i_i8, 0, pubKeyLen * sizeof(int8_t));
      memcpy(value->sourcePublicKey.m256i_i8, srcPubKey, pubKeyLen * sizeof(int8_t));
      memcpy(value->destinationPublicKey.m256i_i8, dstPubKey, pubKeyLen * sizeof(int8_t));

      //Populate Numeric Values of Transaction
      deserialize_fixed_int((int64_t*)&value->amount, &bytes[byteOffset], 8, true);
      byteOffset += 8;
      deserialize_fixed_int((int32_t*)&value->tick, &bytes[byteOffset], 4, true);
      byteOffset += 4;
      deserialize_fixed_int((int16_t*)&value->inputType, &bytes[byteOffset], 2, true);
      byteOffset += 2;
      deserialize_fixed_int((int16_t*)&value->inputSize, &bytes[byteOffset], 2, true);

      //Done
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

