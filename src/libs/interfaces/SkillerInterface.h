
/***************************************************************************
 *  SkillerInterface.h - Fawkes BlackBoard Interface - SkillerInterface
 *
 *  Templated created:   Thu Oct 12 10:49:19 2006
 *  Copyright  2008-2010  Tim Niemueller
 *
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version. A runtime exception applies to
 *  this software (see LICENSE.GPL_WRE file mentioned below for details).
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL_WRE file in the doc directory.
 */

#ifndef __INTERFACES_SKILLERINTERFACE_H_
#define __INTERFACES_SKILLERINTERFACE_H_

#include <interface/interface.h>
#include <interface/message.h>
#include <interface/field_iterator.h>

namespace fawkes {

class SkillerInterface : public Interface
{
 /// @cond INTERNALS
 INTERFACE_MGMT_FRIENDS(SkillerInterface)
 /// @endcond
 public:
  /* constants */

  /** 
	This determines the current status of skill execution.
       */
  typedef enum {
    S_INACTIVE /**< No skill is running. */,
    S_FINAL /**< The skill string has been successfully processed. */,
    S_RUNNING /**< The execution is still running. */,
    S_FAILED /**< The execution failed and cannot succeed anymore. */
  } SkillStatusEnum;
  const char * tostring_SkillStatusEnum(SkillStatusEnum value) const;

 private:
#pragma pack(push,4)
  /** Internal data storage, do NOT modify! */
  typedef struct {
    int64_t timestamp_sec;  /**< Interface Unix timestamp, seconds */
    int64_t timestamp_usec; /**< Interface Unix timestamp, micro-seconds */
    char skill_string[1024]; /**< 
      Combined string showing as much as possible of the currently
      executed skills string, at most the first 1023 bytes of it.
      Must be properly null-terminated. Shall be of the following
      format: S skill1(param) || S skill2() [||...]  where S is one of
      F, R, X (marking the skill state, where X is failure) and
      skill1(param) etc. are the skills of the respective
      sandboxes. || is used to separate the strings (representing
      concurrent execution). Inactive channels shall be omitted.
     */
    char error[128]; /**< 
      String describing the error. Can be set by a skill when it fails.
      Shall be of the form: N string | N string...
      where N denotes the skill channel and string the respective
      error string. Active channels which did not post an error
      message shall be omitted.
     */
    uint32_t exclusive_controller; /**< 
      Instance serial of the exclusive controller of the skiller. If this does not
      carry your instance serial your exec messages will be ignored. Aquire control with
      the AquireControlMessage. Make sure you release control before exiting.
     */
    uint32_t msgid[8]; /**< 
      The IDs of the messages that are currently being processed,
      or 0 if no message is being processed.
     */
    int32_t status[8]; /**< 
      The status of the current skill execution for the appropriate channel.
     */
  } SkillerInterface_data_t;
#pragma pack(pop)

  SkillerInterface_data_t *data;

  interface_enum_map_t enum_map_SkillStatusEnum;
 public:
  /* messages */
  class ExecSkillMessage : public Message
  {
   private:
#pragma pack(push,4)
    /** Internal data storage, do NOT modify! */
    typedef struct {
      int64_t timestamp_sec;  /**< Interface Unix timestamp, seconds */
      int64_t timestamp_usec; /**< Interface Unix timestamp, micro-seconds */
      char skill_string[1024]; /**< 
      Combined string showing as much as possible of the currently
      executed skills string, at most the first 1023 bytes of it.
      Must be properly null-terminated. Shall be of the following
      format: S skill1(param) || S skill2() [||...]  where S is one of
      F, R, X (marking the skill state, where X is failure) and
      skill1(param) etc. are the skills of the respective
      sandboxes. || is used to separate the strings (representing
      concurrent execution). Inactive channels shall be omitted.
     */
    } ExecSkillMessage_data_t;
#pragma pack(pop)

    ExecSkillMessage_data_t *data;

  interface_enum_map_t enum_map_SkillStatusEnum;
   public:
    ExecSkillMessage(const char * ini_skill_string);
    ExecSkillMessage();
    ~ExecSkillMessage();

    ExecSkillMessage(const ExecSkillMessage *m);
    /* Methods */
    char * skill_string() const;
    void set_skill_string(const char * new_skill_string);
    size_t maxlenof_skill_string() const;
    virtual Message * clone() const;
  };

  class RestartInterpreterMessage : public Message
  {
   private:
#pragma pack(push,4)
    /** Internal data storage, do NOT modify! */
    typedef struct {
      int64_t timestamp_sec;  /**< Interface Unix timestamp, seconds */
      int64_t timestamp_usec; /**< Interface Unix timestamp, micro-seconds */
    } RestartInterpreterMessage_data_t;
#pragma pack(pop)

    RestartInterpreterMessage_data_t *data;

  interface_enum_map_t enum_map_SkillStatusEnum;
   public:
    RestartInterpreterMessage();
    ~RestartInterpreterMessage();

    RestartInterpreterMessage(const RestartInterpreterMessage *m);
    /* Methods */
    virtual Message * clone() const;
  };

  class StopExecMessage : public Message
  {
   private:
#pragma pack(push,4)
    /** Internal data storage, do NOT modify! */
    typedef struct {
      int64_t timestamp_sec;  /**< Interface Unix timestamp, seconds */
      int64_t timestamp_usec; /**< Interface Unix timestamp, micro-seconds */
      uint32_t channel; /**< Which channel to stop. */
    } StopExecMessage_data_t;
#pragma pack(pop)

    StopExecMessage_data_t *data;

  interface_enum_map_t enum_map_SkillStatusEnum;
   public:
    StopExecMessage(const uint32_t ini_channel);
    StopExecMessage();
    ~StopExecMessage();

    StopExecMessage(const StopExecMessage *m);
    /* Methods */
    uint32_t channel() const;
    void set_channel(const uint32_t new_channel);
    size_t maxlenof_channel() const;
    virtual Message * clone() const;
  };

  class StopAllMessage : public Message
  {
   private:
#pragma pack(push,4)
    /** Internal data storage, do NOT modify! */
    typedef struct {
      int64_t timestamp_sec;  /**< Interface Unix timestamp, seconds */
      int64_t timestamp_usec; /**< Interface Unix timestamp, micro-seconds */
    } StopAllMessage_data_t;
#pragma pack(pop)

    StopAllMessage_data_t *data;

  interface_enum_map_t enum_map_SkillStatusEnum;
   public:
    StopAllMessage();
    ~StopAllMessage();

    StopAllMessage(const StopAllMessage *m);
    /* Methods */
    virtual Message * clone() const;
  };

  class AcquireControlMessage : public Message
  {
   private:
#pragma pack(push,4)
    /** Internal data storage, do NOT modify! */
    typedef struct {
      int64_t timestamp_sec;  /**< Interface Unix timestamp, seconds */
      int64_t timestamp_usec; /**< Interface Unix timestamp, micro-seconds */
      bool steal_control; /**< 
      If set to true steal the control from someone else who has it
      atm. Use this with caution. But sometimes it is necessary to
      ensure a successful operation, e.g. if the agent tries to
      acquire control.
     */
    } AcquireControlMessage_data_t;
#pragma pack(pop)

    AcquireControlMessage_data_t *data;

  interface_enum_map_t enum_map_SkillStatusEnum;
   public:
    AcquireControlMessage(const bool ini_steal_control);
    AcquireControlMessage();
    ~AcquireControlMessage();

    AcquireControlMessage(const AcquireControlMessage *m);
    /* Methods */
    bool is_steal_control() const;
    void set_steal_control(const bool new_steal_control);
    size_t maxlenof_steal_control() const;
    virtual Message * clone() const;
  };

  class ReleaseControlMessage : public Message
  {
   private:
#pragma pack(push,4)
    /** Internal data storage, do NOT modify! */
    typedef struct {
      int64_t timestamp_sec;  /**< Interface Unix timestamp, seconds */
      int64_t timestamp_usec; /**< Interface Unix timestamp, micro-seconds */
    } ReleaseControlMessage_data_t;
#pragma pack(pop)

    ReleaseControlMessage_data_t *data;

  interface_enum_map_t enum_map_SkillStatusEnum;
   public:
    ReleaseControlMessage();
    ~ReleaseControlMessage();

    ReleaseControlMessage(const ReleaseControlMessage *m);
    /* Methods */
    virtual Message * clone() const;
  };

  virtual bool message_valid(const Message *message) const;
 private:
  SkillerInterface();
  ~SkillerInterface();

 public:
  /* Methods */
  char * skill_string() const;
  void set_skill_string(const char * new_skill_string);
  size_t maxlenof_skill_string() const;
  char * error() const;
  void set_error(const char * new_error);
  size_t maxlenof_error() const;
  uint32_t exclusive_controller() const;
  void set_exclusive_controller(const uint32_t new_exclusive_controller);
  size_t maxlenof_exclusive_controller() const;
  uint32_t * msgid() const;
  uint32_t msgid(unsigned int index) const;
  void set_msgid(unsigned int index, const uint32_t new_msgid);
  void set_msgid(const uint32_t * new_msgid);
  size_t maxlenof_msgid() const;
  SkillStatusEnum * status() const;
  SkillStatusEnum status(unsigned int index) const;
  void set_status(unsigned int index, const SkillStatusEnum new_status);
  void set_status(const SkillStatusEnum * new_status);
  size_t maxlenof_status() const;
  virtual Message * create_message(const char *type) const;

  virtual void copy_values(const Interface *other);
  virtual const char * enum_tostring(const char *enumtype, int val) const;

};

} // end namespace fawkes

#endif
