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
// Author:      Adrian Schuur, schuur@de.ibm.com
//
// Modified By: Adrian Duta
//              Mark Hamzy,    hamzy@us.ibm.com
//
//%/////////////////////////////////////////////////////////////////////////////
package org.pegasus.jmpi;

import java.util.*;

/**
    Creates and instantiates an instance of a CIM class.  Use this interface
    to describe a managed object that belongs to a particular class.
    Instances contain actual data.  Clients use CIMInstance in conjunction
    with the CIMClient instance methods like createInstance or setInstance
    to manipulate instances within a namespace.
 */
public class CIMInstance
             implements CIMElement
{
    private int    cInst;
    private String name;

    private native int    _new              ();
    private native int    _newCn            (String   n);
    private native int    _filterProperties (int      cInst,
                                             String[] pl,
                                             boolean  iq,
                                             boolean  ic,
                                             boolean  lo);
    private native void   _setName          (int      cInst,
                                             String   n);
    private native void   _setProperty      (int      cInst,
                                             String   n,
                                             int      vInst);
    private native void   _setProperties    (int      cInst,
                                             Vector   v);
    private native int    _getProperty      (int      cInst,
                                             String   n);
    private native Vector _getKeyValuePairs (int      cInst,
                                             Vector   vec);
    private native Vector _getProperties    (int      cInst,
                                             Vector   vec);
    private native String _getClassName     (int      cInst);
    private native int    _getQualifier     (int      cInst,
                                             String   n);
    private native int    _clone            (int      cInst);
    private native String _toString         (int      cInst);
    private native void   _finalize         (int      ci);
    private native int    _getObjectPath    (int      cInst);
    private native void   _setObjectPath    (int      cInst,
                                             int      ciCop);
    private native int    _getPropertyCount (int      cInst);
    private native int    _getPropertyI     (int      cInst,
                                             int      i);

    protected void finalize ()
    {
        _finalize (cInst);
    }

    CIMInstance (int ci)
    {
        cInst = ci;
    }

    protected int cInst ()
    {
        return cInst;
    }

    public CIMInstance (String cn)
    {
        name  = cn;
        cInst = _newCn (cn);
    }

    public CIMInstance filterProperties (String  propertyList[],
                                         boolean includeQualifier,
                                         boolean includeClassOrigin)
    {
        return new CIMInstance (_filterProperties (cInst, propertyList, includeQualifier, includeClassOrigin, false));
    }

    public CIMInstance localElements ()
    {
        return new CIMInstance (_filterProperties (cInst, null, false, false, true));
    }

    public void setName (String n)
    {
        name = n;

        if (cInst == -1)
            return;

        _setName (cInst, n);
    }

    public String getName ()
    {
        return name;
    }

    /**
         sets property value for the CIM value
         @param String property name to set
         @param CIMValue a CIMProperty value
      */
    public void setProperty(String n, CIMValue v)
       throws CIMException
    {
        /* Fix for 4019 */
        if (cInst == -1)
        {
           throw new CIMException (1, "Invalid CIMInstance");
        }
        if (v.cInst () == -1)
        {
           throw new CIMException (1, "Invalid CIMValue");
        }
        /* Fix for 4019 */

        _setProperty (cInst, n, v.cInst ());
    }

    public void setProperty(Vector v)
    {
        if (cInst == -1)
            return;

        _setProperties (cInst, v);
    }

    /**
         Returns a property as specified by the name.
         @param String name  - name of the property
         @return CIMProperty property object the specified name
      */
    public CIMProperty getProperty (String n)
    {
        if (cInst == -1)
            return null;

        int p = _getProperty (cInst, n);

        if (p != -1)
           return new CIMProperty (p);

        return null;
    }

    /**
        Returns the list of key-value pairs for this instance
        @return Vector  list of key-value pairs for this instance
     */
    public Vector getKeyValuePairs ()
    {
        if (cInst == -1)
            return null;

        return _getKeyValuePairs (cInst, new Vector ());
    }

    /**
        Gets the properties list
        @return Vector  properties list
     */
    public Vector getProperties ()
    {
        if (cInst == -1)
            return null;

        return _getProperties (cInst, new Vector ());
    }

    /**
        Returns the class name of the instance
        @return String with the class name.
     */
    public String getClassName()
    {
        if (cInst == -1)
            return null;

        return _getClassName (cInst);
    }

    /**
        getQualifiers - Retrieves the qualifier object defined for this
                        instance.
        @return Vector  list of qualifier objects for the CIMInstance.
     */
    public CIMQualifier getQualifier(String n)
    {
        if (cInst == -1)
            return null;

        int qInst = _getQualifier(cInst,n);

        if (qInst != -1)
            return new CIMQualifier(qInst);

        return null;
    }

    /**
        Returns a String representation of the CIMInstance.
        @return String representation of the CIMInstance
     */
    public String toString ()
    {
        if (cInst == -1)
            return null;

        Vector       v   = getProperties ();
        StringBuffer str = new StringBuffer ("Instance of "+getClassName()+" {\n");

        for (int i = 0, m = v.size (); i < m; i++)
        {
            CIMProperty cp = (CIMProperty)v.elementAt (i);

            str.append ("  " + cp.toString () + "\n");
        }
        str.append ("};\n");

        return str.toString ();
    }

    public Object clone ()
    {
        if (cInst == -1)
            return null;

        int ciNew = _clone (cInst);

        if (  ciNew != -1
           && ciNew != 0
           )
        {
            return new CIMInstance (ciNew);
        }
        else
        {
            return null;
        }
    }

    public CIMObjectPath getObjectPath ()
    {
        int ciCop = _getObjectPath (cInst);

        if (ciCop != 0)
        {
            return new CIMObjectPath (ciCop);
        }
        else
        {
            return null;
        }
    }

    public void setObjectPath (CIMObjectPath cop)
    {
        _setObjectPath (cInst, cop.cInst ());
    }

    public int getPropertyCount ()
    {
        return _getPropertyCount (cInst);
    }

    public CIMProperty getProperty (int i)
    {
        int ciProperty = _getPropertyI (cInst, i);

        if (ciProperty != 0)
        {
            return new CIMProperty (ciProperty);
        }
        else
        {
            return null;
        }
    }

    static {
       System.loadLibrary("JMPIProviderManager");
    }
}
