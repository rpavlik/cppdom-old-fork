#ifndef XMLPP_PREDICATES
#define XMLPP_PREDICATES

#include <string>
#include <xmlpp/xmlpp.h>

namespace xmlpp
{
   class AttributeExists
   {
   public:
      /** set the attribute name to match. */
      void setName( std::string attrName ) { mName = attrName; }

      bool operator()( const XMLNodePtr& node )
      {
         return node->hasAttribute();
      }
   private:
      std::string mName;
   };

   class AttributeEqualToValue
   {
   public:
      /** set the attribute name to match. */
      void setName( std::string attrName ) { mName = attrName; }

      /** set the attribute value to match. */
      void setValue( std::string val ) { mValue = val; }

      bool operator()( const XMLNodePtr& node )
      {
         // if doesn't have the attribute, then were done.
         if (!node->hasAttribute()) 
         {
            return false;
         }

         return mValue == (std::string)node->getAttribute( mName );
      }   
   private:
      std::string mName, mValue;
   };

} // end xmlpp namespace

#endif
