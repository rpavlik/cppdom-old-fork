#ifndef XMLPP_PREDICATES
#define XMLPP_PREDICATES

#include <string>
#include <xmlpp/xmlpp.h>

namespace xmlpp
{
   class AttributeType
   {
   public:
      /** set the attribute name to match. */
      AttributeType( std::string attrName ) : mName( attrName ) 
      {
      }
   
      /** set the attribute name to match. */
      void setName( std::string attrName ) { mName = attrName; }

      bool operator()( const XMLNodePtr& node )
      {
         return node->hasAttribute( mName );
      }
   private:
      std::string mName;
   };

   class AttributeValue
   {
   public:
      /** set the attribute name to match.
       *  set the attribute value to match. 
       */
      AttributeValue( std::string attrName, std::string val ) : 
         mName( attrName ), mValue( val ) 
      {
      }

      /** set the attribute name to match. */
      void setName( std::string attrName ) { mName = attrName; }

      /** set the attribute value to match. */
      void setValue( std::string val ) { mValue = val; }

      bool operator()( const XMLNodePtr& node )
      {
         // if doesn't have the attribute, then were done.
         if (!node->hasAttribute( mName )) 
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
