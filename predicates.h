#ifndef CPPDOM_PREDICATES
#define CPPDOM_PREDICATES

#include <string>
#include <cppdom/cppdom.h>

namespace cppdom
{
   class HasAttributeNamePredicate
   {
   public:
      /** set the attribute name to match. */
      HasAttributeNamePredicate(const std::string& attrName)
         : mName(attrName)
      {}

      /** set the attribute name to match. */
      void setName(const std::string& attrName) { mName = attrName; }

      bool operator()(const XMLNodePtr& node)
      {
         return node->hasAttribute(mName);
      }

   private:
      std::string mName;
   };

   class HasAttributeValuePredicate
   {
   public:
      /**
       * set the attribute name to match.
       * set the attribute value to match.
       */
      HasAttributeValuePredicate(const std::string& attrName, const std::string&  val)
         : mName(attrName), mValue(val)
      {}

      /** set the attribute name to match. */
      void setName(const std::string& attrName) { mName = attrName; }

      /** set the attribute value to match. */
      void setValue(const std::string& val) { mValue = val; }

      bool operator()(const XMLNodePtr& node)
      {
         // if doesn't have the attribute, then were done.
         if (!node->hasAttribute(mName))
         {
            return false;
         }

         return mValue == (std::string)node->getAttribute(mName);
      }
   private:
      std::string mName
      std::string mValue;
   };
}

#endif
