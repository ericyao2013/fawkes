
----------------------------------------------------------------------------
--  rcll_move_product.lua -- Skill to grab a product from the MPS
--
--  Created: Tue Jun 07 17:44:42 2016
--  Copyright  2016  Till Hofmann
--
----------------------------------------------------------------------------

--  This program is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU Library General Public License for more details.
--
--  Read the full text in the LICENSE.GPL file in the doc directory.

-- Initialize module
module(..., skillenv.module_init)

-- Crucial skill information
name               = "rcll_move_product"
fsm                = SkillHSM:new{name=name, start="START", debug=true}
depends_skills     = {"jaco", "or_object"}
depends_interfaces = {
   --{v = "katanaarm", type = "KatanaInterface"}
   {v = "jacoarm", type = "JacoInterface"}
}

documentation      = [==[Skill to move a product from the MPS to the ground.
]==]

-- Initialize as skill module
skillenv.skill_module(_M)


-- Constants and variables
--local TARGET_REACHED_DISTANCE = 0.8
local PARK_X = 0.0
local PARK_Y = 0.4
local PARK_Z = 1.0
local PARK_E1 = 1.57
local PARK_E2 = 3.14
local PARK_E3 = 3.14
local PROD_POS_X = 0.25
local PROD_POS_Y = 0.20
local PROD_POS_Z = 0.91
local PICK_UP_E3 = 3.4
local PICK_UP_Y_OFFSET = 0.02
local DROP_POS_X = -0.1
local DROP_POS_Y = 0.4
local DROP_POS_Z = 0.3

-- States
fsm:define_states{ export_to=_M,

  {"START", SkillJumpState, final_to="ADD_OBJS", fail_to="FAILED",
    skills={{or_object}}},
  -- add MPS and the product object
  {"ADD_OBJS", SkillJumpState, final_to="MOVE_OBJS", fail_to="FAILED",
    skills={{or_object}}},
  -- stack the product on top of the MPS belt
  {"MOVE_OBJS", SkillJumpState, final_to="INIT_PARK", fail_to="FAILED",
    skills={{or_object}}},
  -- move to the initial park position
  {"INIT_PARK", SkillJumpState, final_to="OPEN_GRIPPER",
    fail_to="RETRACT_TO_HOME", skills={{jaco}}},
  {"OPEN_GRIPPER", SkillJumpState, final_to="HOVER_BELT",
    fail_to="RETRACT_TO_HOME", skills={{jaco}}},
  -- Grabbing the product from the belt works as follows:
  -- 1. Hover over the belt (~10cm above the belt)
  -- 2. Move down
  -- 3. Grab the object
  -- 4. Move up 10cm
  -- 5. Move away
  -- If the hover fails, there is a relaxed hover position further above the
  -- belt.
  -- If moving to the grabbing position fails, try a relaxed grabbing position
  -- further above the belt.
  -- If the grab fails, open the gripper and retract to parking.
  {"HOVER_BELT", SkillJumpState, final_to="BELT_GRAB_POS",
    fail_to="CLOSE_HOVER_BELT", skills={{jaco}}},
  {"CLOSE_HOVER_BELT", SkillJumpState, final_to="BELT_GRAB_POS",
    fail_to="FAILED", skills={{jaco}}},
  {"BELT_GRAB_POS", SkillJumpState, final_to="GRAB_FROM_BELT",
    fail_to="RELAXED_BELT_GRAB_POS", skills={{jaco}}},
  {"RELAXED_BELT_GRAB_POS", SkillJumpState, final_to="GRAB_FROM_BELT",
    fail_to="FAILED", skills={{jaco}}},
  {"GRAB_FROM_BELT", SkillJumpState, final_to="CHECK_GRAB",
    fail_to="RETRACT_OPEN_GRIPPER", skills={{jaco}}},
  {"CHECK_GRAB", JumpState},
  {"ATTACH_PROD", SkillJumpState, final_to="MOVE_UP_FROM_BELT",
    fail_to="RETRACT_OPEN_GRIPPER", skills={{or_object}}},
  {"MOVE_UP_FROM_BELT", SkillJumpState, final_to="MOVE_TO_MIDDLE",
    fail_to="FAILED", skills={{jaco}}},
  -- intermediate position which is above the drop position
  {"MOVE_TO_MIDDLE", SkillJumpState, final_to="HOVER_GROUND",
    fail_to="RETRACT_TO_PARK", skills={{jaco}}},
  -- Dropping the object works as follows:
  -- 1. Hover over the dropping position
  -- 2. Move down to the dropping position
  -- 3. Drop the object
  -- 4. Move back up
  -- 5. Go back to park
  {"HOVER_GROUND", SkillJumpState, final_to="DROP_POS",
    fail_to="RETRACT_TO_PARK", skills={{jaco}}},
  {"DROP_POS", SkillJumpState, final_to="DROP", fail_to="RETRACT_MOVE_UP",
    skills={{jaco}}},
  {"DROP", SkillJumpState, final_to="POST_DROP", fail_to="FAILED",
    skills={{jaco},{or_object}}},
  {"POST_DROP", SkillJumpState, final_to="PARK", fail_to="FAILED",
    skills={{jaco}}},
  {"PARK", SkillJumpState, final_to="FINAL", fail_to="RETRY_PARK_INTERMEDIATE",
    skills={{jaco}}},
  -- failure handling
  {"RETRACT_TO_HOME", SkillJumpState, final_to="FAILED", fail_to="FAILED",
    skills={{jaco}}},
  {"RETRACT_TO_PARK", SkillJumpState, final_to="FAILED",
    fail_to="RETRACT_TO_HOME", skills={{jaco}}},
  {"RETRACT_OPEN_GRIPPER", SkillJumpState, final_to="RETRACT_MOVE_UP_FROM_MPS",
    fail_to="RETRACT_MOVE_UP_FROM_MPS", skills={{jaco}}},
  {"RETRACT_MOVE_UP_FROM_MPS", SkillJumpState, final_to="RETRACT_TO_PARK",
    fail_to="FAILED", skills={{jaco}}},
  {"RETRACT_MOVE_UP", SkillJumpState, final_to="RETRACT_TO_PARK",
    fail_to="FAILED", skills={{jaco}}},
  {"RETRY_PARK_INTERMEDIATE", SkillJumpState, final_to="RETRY_PARK",
    fail_to="FAILED", skills={{jaco}}},
  {"RETRY_PARK", SkillJumpState, final_to="FINAL", fail_to="RETRACT_TO_HOME",
    skills={{jaco}}}
}

-- Transitions
fsm:add_transitions{
  {"CHECK_GRAB", "ATTACH_PROD", cond="self.success"},
  {"CHECK_GRAB", "RETRACT_OPEN_GRIPPER", cond=true}
}

function START:init()
  self.args["or_object"] = {delete_all=true}
end

function ADD_OBJS:init()
  self.args["or_object"] = {
    add={ {name="mps", path="rcll.machine.xml"},
          {name="prod", path="rcll.prod.xml"}}}
end

function MOVE_OBJS:init()
  self.args["or_object"] = {
    move={{name="mps", x=0.55, y=0.6, z=0},
          {name="prod", x=PROD_POS_X,y=PROD_POS_Y,z=PROD_POS_Z}
          }}
end

function INIT_PARK:init()
  self.args["jaco"] = { x=PARK_X,y=PARK_Y,z=PARK_Z,
                        e1=PARK_E1,e2=PARK_E2,e3=PARK_E3}
end

function OPEN_GRIPPER:init()
  self.args["jaco"] = {gripper="open"}
end

function HOVER_BELT:init()
  self.args["jaco"] = {x=PROD_POS_X,y=PROD_POS_Y+PICK_UP_Y_OFFSET,z=PROD_POS_Z+0.1,
                        e1=1.57,e2=3.14,e3=PICK_UP_E3}
end

function CLOSE_HOVER_BELT:init()
  self.args["jaco"] = {x=PROD_POS_X,y=PROD_POS_Y+PICK_UP_Y_OFFSET,z=PROD_POS_Z+0.05,
                        e1=1.57,e2=3.14,e3=PICK_UP_E3}

end

function BELT_GRAB_POS:init()
  self.args["jaco"] = {x=PROD_POS_X,y=PROD_POS_Y+PICK_UP_Y_OFFSET,z=PROD_POS_Z+0.02,
                        e1=1.57,e2=3.14,e3=PICK_UP_E3}
end

function RELAXED_BELT_GRAB_POS:init()
  self.args["jaco"] = {x=PROD_POS_X,y=PROD_POS_Y+PICK_UP_Y_OFFSET,z=PROD_POS_Z+0.05,
                        e1=1.57,e2=3.14,e3=PICK_UP_E3}
end

function GRAB_FROM_BELT:init()
  self.args["jaco"] = {gripper="close"}
end

function ATTACH_PROD:init()
  self.args["or_object"] = {attach={"prod"}}
end


function CHECK_GRAB:init()
  local f1, f2 = jacoarm:finger1(), jacoarm:finger2()
  if f1 <= 30 or f1 >= 50 or f2 <= 30 or f2 >= 50 then
    self.success = false
  else
    self.success = true
  end
end

function MOVE_UP_FROM_BELT:init()
  self.args["jaco"] = {x=PROD_POS_X,y=PROD_POS_Y+PICK_UP_Y_OFFSET,z=PROD_POS_Z+0.2,
                        e1=1.57,e2=3.14,e3=PICK_UP_E3}
end

function MOVE_TO_MIDDLE:init()
  self.args["jaco"] = {x=-0.1, y=0.4, z=0.95,e1=1.57,e2=3.14,e3=PICK_UP_E3}
end

function HOVER_GROUND:init()
  self.args["jaco"] = {x=DROP_POS_X, y=DROP_POS_Y, z=DROP_POS_Z+0.12,
                       e1=1.57,e2=3.14,e3=PICK_UP_E3}
end

function DROP_POS:init()
  self.args["jaco"] = {x=DROP_POS_X, y=DROP_POS_Y, z=DROP_POS_Z+0.02,
                       e1=1.57,e2=3.14,e3=PICK_UP_E3}
end

function DROP:init()
  self.args["jaco"] = {gripper="open"}
  self.args["or_object"] = {release={"prod"}}
end

function POST_DROP:init()
  self.args["jaco"] = {x=DROP_POS_X, y=DROP_POS_Y, z=DROP_POS_Z+0.12,
                       e1=1.57,e2=3.14,e3=PICK_UP_E3}
end

function PARK:init()
  self.args["jaco"] = { x=PARK_X,y=PARK_Y,z=PARK_Z,
                        e1=PARK_E1,e2=PARK_E2,e3=PARK_E3}
end

-- failure handling
function RETRACT_TO_PARK:init()
  self.args["jaco"] = { x=PARK_X,y=PARK_Y,z=PARK_Z,
                        e1=PARK_E1,e2=PARK_E2,e3=PARK_E3}
end

function RETRACT_TO_HOME:init()
  self.args["jaco"] = {pos="home"}
end

function RETRACT_OPEN_GRIPPER:init()
  self.args["jaco"] = {gripper="open"}
end

function RETRACT_MOVE_UP_FROM_MPS:init()
  self.args["jaco"] = {x=PROD_POS_X, y=PROD_POS_Y, z=PROD_POS_Z+0.1,
                        e1=1.57,e2=3.14,e3=PICK_UP_E3}
end

function RETRACT_MOVE_UP:init()
  self.args["jaco"] = {x=-0.1, y=0.4, z=0.2,e1=1.57,e2=3.14,e3=PICK_UP_E3}
end

function RETRY_PARK_INTERMEDIATE:init()
  self.args["jaco"] = {x=-0.1, y=0.4, z=0.4,e1=1.57,e2=3.14,e3=PICK_UP_E3}
end

function RETRY_PARK:init()
  self.args["jaco"] = { x=PARK_X,y=PARK_Y,z=PARK_Z,
                        e1=PARK_E1,e2=PARK_E2,e3=PARK_E3}
end

