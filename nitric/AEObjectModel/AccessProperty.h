/*	================
 *	AccessProperty.h
 *	================
 */

#ifndef AEOBJECTMODEL_ACCESSPROPERTY_H
#define AEOBJECTMODEL_ACCESSPROPERTY_H

#include <map>

#ifndef __AEREGISTRY__
#include <AERegistry.h>
#endif

#ifndef NITROGEN_AEOBJECTS_HH
#include "Nitrogen/AEObjects.hh"
#endif


namespace Nitrogen
{
	
	nucleus::owned< AEDesc_Token > AccessProperty( AEPropertyID         propertyID,
	                                               const AEDesc_Token&  containerToken,
	                                               AEObjectClass        containerClass );
	
	class PropertyAccessor
	{
		public:
			typedef nucleus::owned< AEDesc_Token > (*Callback)( AEPropertyID, const AEDesc_Token&, AEObjectClass );
		
		private:
			typedef std::map< AEPropertyID, Callback >  PropertyMap;
			typedef std::map< DescType, PropertyMap >  Map;
			
			Map map;
			
			// not implemented:
			PropertyAccessor( const PropertyAccessor& );
			PropertyAccessor& operator=( const PropertyAccessor& );
		
		public:
			PropertyAccessor();
			
			void Register( AEPropertyID propertyID, DescType containerType, PropertyAccessor::Callback callback )
			{
				map[ containerType ][ propertyID ] = callback;
			}
			
			Callback FindAccessor( AEPropertyID  propertyID, DescType tokenType );
			
			nucleus::owned< AEDesc_Token > AccessProperty( AEPropertyID         propertyID,
			                                               const AEDesc_Token&  containerToken,
			                                               AEObjectClass        containerClass );
			
			nucleus::owned< AEDesc_Token > AccessAll( const AEDesc_Token&  containerToken,
			                                          AEObjectClass        containerClass );
	};
	
	PropertyAccessor& TheGlobalPropertyAccessor();
	
	inline void RegisterPropertyAccessor( AEPropertyID propertyID, DescType containerType, PropertyAccessor::Callback callback )
	{
		TheGlobalPropertyAccessor().Register( propertyID, containerType, callback );
	}
	
	nucleus::owned< AEDesc_Token > AccessClassProperty( AEPropertyID         /* propertyID */,
	                                                    const AEDesc_Token&  /* containerToken */,
	                                                    AEObjectClass        containerClass );
	
	nucleus::owned< AEDesc_Token > AccessAllProperties( AEPropertyID         propertyID,
	                                                    const AEDesc_Token&  containerToken,
	                                                    AEObjectClass        containerClass );
	
	inline nucleus::owned< AEDesc_Token > AccessAllProperties( const AEDesc_Token&  containerToken,
	                                                           AEObjectClass        containerClass )
	{
		return AccessAllProperties( AEPropertyID(), containerToken, containerClass );
	}
	
}

#endif
