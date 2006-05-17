//%2006////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000, 2001, 2002 BMC Software; Hewlett-Packard Development
// Company, L.P.; IBM Corp.; The Open Group; Tivoli Systems.
// Copyright (c) 2003 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation, The Open Group.
// Copyright (c) 2004 BMC Software; Hewlett-Packard Development Company, L.P.;
// IBM Corp.; EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2005 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; VERITAS Software Corporation; The Open Group.
// Copyright (c) 2006 Hewlett-Packard Development Company, L.P.; IBM Corp.;
// EMC Corporation; Symantec Corporation; The Open Group.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// THE ABOVE COPYRIGHT NOTICE AND THIS PERMISSION NOTICE SHALL BE INCLUDED IN
// ALL COPIES OR SUBSTANTIAL PORTIONS OF THE SOFTWARE. THE SOFTWARE IS PROVIDED
// "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
// LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//==============================================================================
//
// Author:      Mark Hamzy, hamzy@us.ibm.com
//
// Modified By:
//
//%/////////////////////////////////////////////////////////////////////////////
package org.pegasus.jmpi;

public class OperationContext
{
   public final static String IdentityContainer                    = "IdentityContainer";
   //                            String:          userName
   public final static String SubscriptionInstanceContainer        = "SubscriptionInstanceContainer";
   //                            CIMInstance:     subscriptionInstance
   public final static String SubscriptionInstanceNamesContainer   = "SubscriptionInstanceNamesContainer";
   //                            CIMObjectPath[]: subscriptionInstanceNames
   public final static String SubscriptionFilterConditionContainer = "SubscriptionFilterConditionContainer";
   //                            String:          filterCondition
   //                            String:          queryLanguage
   public final static String SubscriptionFilterQueryContainer     = "SubscriptionFilterQueryContainer";
   //                            String:          filterQuery
   //                            String:          queryLanguage
   //                            String:          sourceNameSpace
   public final static String SnmpTrapOidContainer                 = "SnmpTrapOidContainer";
   //                            String:          snmpTrapOid
   public final static String LocaleContainer                      = "LocaleContainer";
   //                            String:          languageId
   public final static String ProviderIdContainer                  = "ProviderIdContainer";
   //                            String:          module
   //                            String:          provider
   //                            Boolean:         isRemoteNameSpace
   //                            String:          remoteInfo
   public final static String CachedClassDefinitionContainer       = "CachedClassDefinitionContainer";
   //                            CIMClass:        class

   private int cInst;

   private native Object _get (int cInst, String container, String key);

   protected int cInst ()
   {
      return cInst;
   }

   public OperationContext (int cInst)
   {
      this.cInst = cInst;
   }

   public void unassociate ()
   {
      cInst = 0;
   }

   public Object get (String container,
                      String key)
      throws CIMException
   {
      if (cInst != 0)
      {
         return _get (cInst, container, key);
      }

      return null;
   }

   static {
       System.loadLibrary ("JMPIProviderManager");
   }
}
