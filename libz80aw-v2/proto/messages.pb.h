// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: messages.proto

#ifndef PROTOBUF_INCLUDED_messages_2eproto
#define PROTOBUF_INCLUDED_messages_2eproto

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3006001
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#define PROTOBUF_INTERNAL_EXPORT_protobuf_messages_2eproto 

namespace protobuf_messages_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[2];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
void AddDescriptors();
}  // namespace protobuf_messages_2eproto
class Reply;
class ReplyDefaultTypeInternal;
extern ReplyDefaultTypeInternal _Reply_default_instance_;
class Request;
class RequestDefaultTypeInternal;
extern RequestDefaultTypeInternal _Request_default_instance_;
namespace google {
namespace protobuf {
template<> ::Reply* Arena::CreateMaybeMessage<::Reply>(Arena*);
template<> ::Request* Arena::CreateMaybeMessage<::Request>(Arena*);
}  // namespace protobuf
}  // namespace google

enum Request_RequestType {
  Request_RequestType_FREE_MEM = 0,
  Request_RequestType_Request_RequestType_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  Request_RequestType_Request_RequestType_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool Request_RequestType_IsValid(int value);
const Request_RequestType Request_RequestType_RequestType_MIN = Request_RequestType_FREE_MEM;
const Request_RequestType Request_RequestType_RequestType_MAX = Request_RequestType_FREE_MEM;
const int Request_RequestType_RequestType_ARRAYSIZE = Request_RequestType_RequestType_MAX + 1;

const ::google::protobuf::EnumDescriptor* Request_RequestType_descriptor();
inline const ::std::string& Request_RequestType_Name(Request_RequestType value) {
  return ::google::protobuf::internal::NameOfEnum(
    Request_RequestType_descriptor(), value);
}
inline bool Request_RequestType_Parse(
    const ::std::string& name, Request_RequestType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<Request_RequestType>(
    Request_RequestType_descriptor(), name, value);
}
// ===================================================================

class Request : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:Request) */ {
 public:
  Request();
  virtual ~Request();

  Request(const Request& from);

  inline Request& operator=(const Request& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  Request(Request&& from) noexcept
    : Request() {
    *this = ::std::move(from);
  }

  inline Request& operator=(Request&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const Request& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Request* internal_default_instance() {
    return reinterpret_cast<const Request*>(
               &_Request_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  void Swap(Request* other);
  friend void swap(Request& a, Request& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline Request* New() const final {
    return CreateMaybeMessage<Request>(NULL);
  }

  Request* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<Request>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const Request& from);
  void MergeFrom(const Request& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(Request* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  typedef Request_RequestType RequestType;
  static const RequestType FREE_MEM =
    Request_RequestType_FREE_MEM;
  static inline bool RequestType_IsValid(int value) {
    return Request_RequestType_IsValid(value);
  }
  static const RequestType RequestType_MIN =
    Request_RequestType_RequestType_MIN;
  static const RequestType RequestType_MAX =
    Request_RequestType_RequestType_MAX;
  static const int RequestType_ARRAYSIZE =
    Request_RequestType_RequestType_ARRAYSIZE;
  static inline const ::google::protobuf::EnumDescriptor*
  RequestType_descriptor() {
    return Request_RequestType_descriptor();
  }
  static inline const ::std::string& RequestType_Name(RequestType value) {
    return Request_RequestType_Name(value);
  }
  static inline bool RequestType_Parse(const ::std::string& name,
      RequestType* value) {
    return Request_RequestType_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  // .Request.RequestType type = 1;
  void clear_type();
  static const int kTypeFieldNumber = 1;
  ::Request_RequestType type() const;
  void set_type(::Request_RequestType value);

  // @@protoc_insertion_point(class_scope:Request)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  int type_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_messages_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class Reply : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:Reply) */ {
 public:
  Reply();
  virtual ~Reply();

  Reply(const Reply& from);

  inline Reply& operator=(const Reply& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  Reply(Reply&& from) noexcept
    : Reply() {
    *this = ::std::move(from);
  }

  inline Reply& operator=(Reply&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const Reply& default_instance();

  enum PayloadCase {
    kFreeMem = 4,
    PAYLOAD_NOT_SET = 0,
  };

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Reply* internal_default_instance() {
    return reinterpret_cast<const Reply*>(
               &_Reply_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  void Swap(Reply* other);
  friend void swap(Reply& a, Reply& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline Reply* New() const final {
    return CreateMaybeMessage<Reply>(NULL);
  }

  Reply* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<Reply>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const Reply& from);
  void MergeFrom(const Reply& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(Reply* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // repeated string info = 2;
  int info_size() const;
  void clear_info();
  static const int kInfoFieldNumber = 2;
  const ::std::string& info(int index) const;
  ::std::string* mutable_info(int index);
  void set_info(int index, const ::std::string& value);
  #if LANG_CXX11
  void set_info(int index, ::std::string&& value);
  #endif
  void set_info(int index, const char* value);
  void set_info(int index, const char* value, size_t size);
  ::std::string* add_info();
  void add_info(const ::std::string& value);
  #if LANG_CXX11
  void add_info(::std::string&& value);
  #endif
  void add_info(const char* value);
  void add_info(const char* value, size_t size);
  const ::google::protobuf::RepeatedPtrField< ::std::string>& info() const;
  ::google::protobuf::RepeatedPtrField< ::std::string>* mutable_info();

  // string error = 3;
  void clear_error();
  static const int kErrorFieldNumber = 3;
  const ::std::string& error() const;
  void set_error(const ::std::string& value);
  #if LANG_CXX11
  void set_error(::std::string&& value);
  #endif
  void set_error(const char* value);
  void set_error(const char* value, size_t size);
  ::std::string* mutable_error();
  ::std::string* release_error();
  void set_allocated_error(::std::string* error);

  // bool success = 1;
  void clear_success();
  static const int kSuccessFieldNumber = 1;
  bool success() const;
  void set_success(bool value);

  // uint32 free_mem = 4;
  private:
  bool has_free_mem() const;
  public:
  void clear_free_mem();
  static const int kFreeMemFieldNumber = 4;
  ::google::protobuf::uint32 free_mem() const;
  void set_free_mem(::google::protobuf::uint32 value);

  void clear_payload();
  PayloadCase payload_case() const;
  // @@protoc_insertion_point(class_scope:Reply)
 private:
  void set_has_free_mem();

  inline bool has_payload() const;
  inline void clear_has_payload();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::RepeatedPtrField< ::std::string> info_;
  ::google::protobuf::internal::ArenaStringPtr error_;
  bool success_;
  union PayloadUnion {
    PayloadUnion() {}
    ::google::protobuf::uint32 free_mem_;
  } payload_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  ::google::protobuf::uint32 _oneof_case_[1];

  friend struct ::protobuf_messages_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// Request

// .Request.RequestType type = 1;
inline void Request::clear_type() {
  type_ = 0;
}
inline ::Request_RequestType Request::type() const {
  // @@protoc_insertion_point(field_get:Request.type)
  return static_cast< ::Request_RequestType >(type_);
}
inline void Request::set_type(::Request_RequestType value) {
  
  type_ = value;
  // @@protoc_insertion_point(field_set:Request.type)
}

// -------------------------------------------------------------------

// Reply

// bool success = 1;
inline void Reply::clear_success() {
  success_ = false;
}
inline bool Reply::success() const {
  // @@protoc_insertion_point(field_get:Reply.success)
  return success_;
}
inline void Reply::set_success(bool value) {
  
  success_ = value;
  // @@protoc_insertion_point(field_set:Reply.success)
}

// repeated string info = 2;
inline int Reply::info_size() const {
  return info_.size();
}
inline void Reply::clear_info() {
  info_.Clear();
}
inline const ::std::string& Reply::info(int index) const {
  // @@protoc_insertion_point(field_get:Reply.info)
  return info_.Get(index);
}
inline ::std::string* Reply::mutable_info(int index) {
  // @@protoc_insertion_point(field_mutable:Reply.info)
  return info_.Mutable(index);
}
inline void Reply::set_info(int index, const ::std::string& value) {
  // @@protoc_insertion_point(field_set:Reply.info)
  info_.Mutable(index)->assign(value);
}
#if LANG_CXX11
inline void Reply::set_info(int index, ::std::string&& value) {
  // @@protoc_insertion_point(field_set:Reply.info)
  info_.Mutable(index)->assign(std::move(value));
}
#endif
inline void Reply::set_info(int index, const char* value) {
  GOOGLE_DCHECK(value != NULL);
  info_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set_char:Reply.info)
}
inline void Reply::set_info(int index, const char* value, size_t size) {
  info_.Mutable(index)->assign(
    reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:Reply.info)
}
inline ::std::string* Reply::add_info() {
  // @@protoc_insertion_point(field_add_mutable:Reply.info)
  return info_.Add();
}
inline void Reply::add_info(const ::std::string& value) {
  info_.Add()->assign(value);
  // @@protoc_insertion_point(field_add:Reply.info)
}
#if LANG_CXX11
inline void Reply::add_info(::std::string&& value) {
  info_.Add(std::move(value));
  // @@protoc_insertion_point(field_add:Reply.info)
}
#endif
inline void Reply::add_info(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  info_.Add()->assign(value);
  // @@protoc_insertion_point(field_add_char:Reply.info)
}
inline void Reply::add_info(const char* value, size_t size) {
  info_.Add()->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_add_pointer:Reply.info)
}
inline const ::google::protobuf::RepeatedPtrField< ::std::string>&
Reply::info() const {
  // @@protoc_insertion_point(field_list:Reply.info)
  return info_;
}
inline ::google::protobuf::RepeatedPtrField< ::std::string>*
Reply::mutable_info() {
  // @@protoc_insertion_point(field_mutable_list:Reply.info)
  return &info_;
}

// string error = 3;
inline void Reply::clear_error() {
  error_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Reply::error() const {
  // @@protoc_insertion_point(field_get:Reply.error)
  return error_.GetNoArena();
}
inline void Reply::set_error(const ::std::string& value) {
  
  error_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:Reply.error)
}
#if LANG_CXX11
inline void Reply::set_error(::std::string&& value) {
  
  error_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:Reply.error)
}
#endif
inline void Reply::set_error(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  error_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:Reply.error)
}
inline void Reply::set_error(const char* value, size_t size) {
  
  error_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:Reply.error)
}
inline ::std::string* Reply::mutable_error() {
  
  // @@protoc_insertion_point(field_mutable:Reply.error)
  return error_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Reply::release_error() {
  // @@protoc_insertion_point(field_release:Reply.error)
  
  return error_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Reply::set_allocated_error(::std::string* error) {
  if (error != NULL) {
    
  } else {
    
  }
  error_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), error);
  // @@protoc_insertion_point(field_set_allocated:Reply.error)
}

// uint32 free_mem = 4;
inline bool Reply::has_free_mem() const {
  return payload_case() == kFreeMem;
}
inline void Reply::set_has_free_mem() {
  _oneof_case_[0] = kFreeMem;
}
inline void Reply::clear_free_mem() {
  if (has_free_mem()) {
    payload_.free_mem_ = 0u;
    clear_has_payload();
  }
}
inline ::google::protobuf::uint32 Reply::free_mem() const {
  // @@protoc_insertion_point(field_get:Reply.free_mem)
  if (has_free_mem()) {
    return payload_.free_mem_;
  }
  return 0u;
}
inline void Reply::set_free_mem(::google::protobuf::uint32 value) {
  if (!has_free_mem()) {
    clear_payload();
    set_has_free_mem();
  }
  payload_.free_mem_ = value;
  // @@protoc_insertion_point(field_set:Reply.free_mem)
}

inline bool Reply::has_payload() const {
  return payload_case() != PAYLOAD_NOT_SET;
}
inline void Reply::clear_has_payload() {
  _oneof_case_[0] = PAYLOAD_NOT_SET;
}
inline Reply::PayloadCase Reply::payload_case() const {
  return Reply::PayloadCase(_oneof_case_[0]);
}
#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)


namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::Request_RequestType> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::Request_RequestType>() {
  return ::Request_RequestType_descriptor();
}

}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_INCLUDED_messages_2eproto
