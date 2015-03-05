/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _REGISTRY_H
#define _REGISTRY_H

//PS make Registry an IFAObject --> NotCloned to have clean handling of Anything
#include "IFAConfObject.h"

//!an infrastructure class to register and retrieve objects by name
//!objects of given types can be registered according to a specification and a InstallerPolicy object.<br>
//!After installation they are available by name. The <i>"type"</i> of the objects is defined by the registry name;<br>
//!installed objects should be "subtypes" of this type (instances of this class or a subclass).<br>
//!the objects are removed from the registry by a termination policy.
class Registry : public NotCloned {
	friend class RegistryIterator;

public:
	//!standard named object constructor
	Registry(const char *category);

	//!deletes internal table
	~Registry();

	/*! Installs objects into registry driven by installerSpec using InstallerPolicy ip. It creates registry entries according to the installer policy passed in.
		\param installerSpec an Anything defining the Objects aliasing or hierarchy structure which should be installed
		\param ip the InstallerPolicy that interprets the installerSpec; it creates and installs objects in the registry as necessary
		\return true in case installation was successful
		\note No mutex is set to protect concurrent access. Exclusive access has to be guaranteed by caller! */
	bool Install(const ROAnything installerSpec, InstallerPolicy *ip);

	//!cleans up registry according to the TerminationPolicy supplied; the registry is no longer usable after that
	//! \param tp the TerminationPolicy that implements the clean up process, since objects can be registered under several names you can't delete them directly
	//! \return returns always true
	bool Terminate(TerminationPolicy *tp);

	//!installs the object o under name in the registry; after that in can be retrieved with Find. No checks against duplicate installs are made
	//! \param name name used for registration
	//! \param o the object to be registered
	void RegisterRegisterableObject(const char *name, RegisterableObject *o);

	//!removes the object associated with name (if any) from the registry, it does not delete anything. After that it is no longer accessible under name
	//! \param name name used for registration
	void UnregisterRegisterableObject(const char *name);

	//! Remove registered aliases for given RegisterableObject
	//! \param obj RegisterableObject to be used for deregistering aliases
	void RemoveAliases(RegisterableObject *obj);

	RegisterableObject *Find(const char *name);

protected:
	//!accessor to the registry's representation
	Anything &GetTable();
	//!Anything holding the registered objects for this registry s category
	Anything fTable;

private:
	Registry(const Registry &);
	Registry &operator=(const Registry &);
};

#include "singleton.hpp"
class MetaRegistryImpl {
	//! Global container holding any registry entries
	Anything fRegistryArray;
	//!delete global registry
	void FinalizeRegArray();
public:
	MetaRegistryImpl();
	~MetaRegistryImpl();
	/*! Returns the registry for category; creates it if it is not already there.
		\param category Name for the registry; usually a name of the 'managing' object used to initialize/finalize belonging objects
		\return Registry object for the category/group given. If the object does not yet exist, a new one will be created
		\note Create registries before starting threads which could possibly use it. For performance reasons, this was intentionally left out! */
	Registry *GetRegistry(const char *category);

	//!creates a registry and inserts it into the global registry table
	//! \param category the name for the registry; usually a class name
	//! \return a newly created Registry object
	Registry *MakeRegistry(const char *category);

	//!removes the registry associated with category from the registry table
	//! \param category the name for the registry; usually a class name
	//! \return a removed Registry object
	Registry *RemoveRegistry(const char *category);

	//!accessor to the global registry table
	Anything &GetRegTable();
};
typedef coast::utility::singleton_default<MetaRegistryImpl> MetaRegistry;

//!simple non-robust, non mt-safe iterator over registry
//! this iterator is neither robust against changes in the underlying
//! data structur, nor threadsafe
//! if you want to delete elements iterate backwards
//! if you expect concurrent accesses, use Mutexes
class RegistryIterator
{
public:
	//!constructor parametrizing the iterator with the registry and the direction of the iteration
	RegistryIterator(Registry *rg, bool forward = true);
	//!destructor does nothing
	~RegistryIterator();

	//!checks boundary condition
	bool HasMore();
	//!delivers next element if not out of bounds
	//! \param key sets the key of the element if any
	//! \return returns the next object if any
	RegisterableObject *Next(String &key);
	//!removes next element if not out of bounds
	//! \param key sets the key of the element if any
	//! \return returns the next object if any
	RegisterableObject *RemoveNext(String &key);

protected:
	//!the registry we iterate over
	Registry *fRegistry;
	//!the direction flag
	bool fForward;
	//!the position of the index
	long fStart;
	long fEnd;
};

#endif
