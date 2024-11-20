// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/files/shares/share_options.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  const ShareAudience ShareAudience::DefaultAudience(_internal::StorageDefaultAudience);

  namespace Models {
    namespace {
      RolePermissions ParseOctal(char c)
      {
        RolePermissions permissions = RolePermissions::None;
        int permissionValue = c - '0';
        if (permissionValue < 0 || permissionValue > 7)
        {
          throw std::invalid_argument("Permission value must be >0 and <7.");
        }
        if ((permissionValue & 4) == 4)
        {
          permissions = permissions | RolePermissions::Read;
        }
        if ((permissionValue & 2) == 2)
        {
          permissions = permissions | RolePermissions::Read;
        }
        if ((permissionValue & 1) == 1)
        {
          permissions = permissions | RolePermissions::Read;
        }
        return permissions;
      }

      char ToOctal(RolePermissions permissions)
      {
        char c = '0';
        if ((permissions & RolePermissions::Read) == RolePermissions::Read)
        {
          c += 4;
        }
        if ((permissions & RolePermissions::Write) == RolePermissions::Write)
        {
          c += 2;
        }
        if ((permissions & RolePermissions::Execute) == RolePermissions::Execute)
        {
          c += 1;
        }
        return c;
      }

      RolePermissions ParseSymbolic(std::string s, bool& setSticky)
      {
        if (s.length() != 3)
        {
          throw std::invalid_argument("Symbolic role permission format is invalid.");
        }

        RolePermissions permissions = RolePermissions::None;
        setSticky = false;
        // Read character
        if (s[0] == 'r')
        {
          permissions = permissions | RolePermissions ::Read;
        }
        else if (s[0] != '-')
        {
          throw std::invalid_argument(
              "Invalid character in symbolic role permission: " + std::string(1, s[0]));
        }

        // Write character
        if (s[1] == 'w')
        {
          permissions = permissions | RolePermissions ::Write;
        }
        else if (s[1] != '-')
        {
          throw std::invalid_argument(
              "Invalid character in symbolic role permission: " + std::string(1, s[1]));
        }

        // Execute character
        if (s[2] == 'x' || s[2] == 's' || s[2] == 't')
        {
          permissions = permissions | RolePermissions ::Execute;
          if (s[2] == 's' || s[2] == 't')
          {
            setSticky = true;
          }
        }
        if (s[2] == 'S' || s[2] == 'T')
        {
          setSticky = true;
        }

        if (s[2] != 'x' && s[2] != 's' && s[2] != 'S' && s[2] != 't' && s[2] != 'T' && s[2] != '-')
        {
          throw std::invalid_argument(
              "Invalid character in symbolic role permission: " + std::string(1, s[2]));
        }

        return permissions;
      }

      std::string ToSymbolic(RolePermissions permissions)
      {
        std::string s;
        s.push_back((permissions & RolePermissions::Read) == RolePermissions::Read ? 'r' : '-');
        s.push_back((permissions & RolePermissions::Write) == RolePermissions::Write ? 'w' : '-');
        s.push_back(
            (permissions & RolePermissions::Execute) == RolePermissions::Execute ? 'x' : '-');
        return s;
      }
    } // namespace

    std::string NfsFileMode::ToOctalFileMode() const
    {
      int higherOrderDigit = 0;
      if (EffectiveUserIdentity)
      {
        higherOrderDigit |= 4;
      }
      if (EffectiveGroupIdentity)
      {
        higherOrderDigit |= 2;
      }
      if (StickyBit)
      {
        higherOrderDigit |= 1;
      }

      std::string modeString = "";
      modeString.push_back(static_cast<char>(higherOrderDigit + '0'));
      modeString.push_back(ToOctal(Owner));
      modeString.push_back(ToOctal(Group));
      modeString.push_back(ToOctal(Other));
      return modeString;
    }

    std::string NfsFileMode::ToSymbolicFileMode() const
    {
      std::string modeString = "";
      modeString.append(ToSymbolic(Owner));
      modeString.append(ToSymbolic(Group));
      modeString.append(ToSymbolic(Other));
      if (EffectiveUserIdentity)
      {
        if (modeString[2] == 'x')
        {
          modeString[2] = 's';
        }
        else
        {
          modeString[2] = 'S';
        }
      }

      if (EffectiveGroupIdentity)
      {
        if (modeString[5] == 'x')
        {
          modeString[5] = 's';
        }
        else
        {
          modeString[5] = 'S';
        }
      }

      if (StickyBit)
      {
        if (modeString[8] == 'x')
        {
          modeString[8] = 't';
        }
        else
        {
          modeString[8] = 'T';
        }
      }
      return modeString;
    }

    NfsFileMode NfsFileMode::ParseOctalFileMode(const std::string& modeString)
    {
      if (modeString.length() != 4)
      {
        throw std::invalid_argument("modeString must be a 4-digit octal number.");
      }
      NfsFileMode mode;
      mode.Owner = ParseOctal(modeString[1]);
      mode.Group = ParseOctal(modeString[2]);
      mode.Other = ParseOctal(modeString[3]);

      int value = modeString[0] - '0';
      if ((value & 4) == 4)
      {
        mode.EffectiveUserIdentity = true;
      }

      if ((value & 2) == 2)
      {
        mode.EffectiveGroupIdentity = true;
      }

      if ((value & 1) == 1)
      {
        mode.StickyBit = true;
      }
      return mode;
    }

    NfsFileMode NfsFileMode::ParseSymbolicFileMode(const std::string& modeString)
    {
      if (modeString.length() != 9)
      {
        throw std::invalid_argument("modeString must be a 9-character octal number.");
      }
      NfsFileMode mode;
      mode.Owner = ParseSymbolic(modeString.substr(0, 3), mode.EffectiveUserIdentity);
      mode.Group = ParseSymbolic(modeString.substr(3, 3), mode.EffectiveGroupIdentity);
      mode.Other = ParseSymbolic(modeString.substr(6, 3), mode.StickyBit);
      return mode;
    }
  } // namespace Models

}}}} // namespace Azure::Storage::Files::Shares
