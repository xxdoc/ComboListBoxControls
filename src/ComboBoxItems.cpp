// ComboBoxItems.cpp: Manages a collection of ComboBoxItem objects

#include "stdafx.h"
#include "ComboBoxItems.h"
#include "ClassFactory.h"


//////////////////////////////////////////////////////////////////////
// implementation of ISupportErrorInfo
STDMETHODIMP ComboBoxItems::InterfaceSupportsErrorInfo(REFIID interfaceToCheck)
{
	if(InlineIsEqualGUID(IID_IComboBoxItems, interfaceToCheck)) {
		return S_OK;
	}
	return S_FALSE;
}
// implementation of ISupportErrorInfo
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// implementation of IEnumVARIANT
STDMETHODIMP ComboBoxItems::Clone(IEnumVARIANT** ppEnumerator)
{
	ATLASSERT_POINTER(ppEnumerator, LPENUMVARIANT);
	if(!ppEnumerator) {
		return E_POINTER;
	}

	*ppEnumerator = NULL;
	CComObject<ComboBoxItems>* pCBoxItemsObj = NULL;
	CComObject<ComboBoxItems>::CreateInstance(&pCBoxItemsObj);
	pCBoxItemsObj->AddRef();

	// clone all settings
	properties.CopyTo(&pCBoxItemsObj->properties);

	pCBoxItemsObj->QueryInterface(IID_IEnumVARIANT, reinterpret_cast<LPVOID*>(ppEnumerator));
	pCBoxItemsObj->Release();
	return S_OK;
}

STDMETHODIMP ComboBoxItems::Next(ULONG numberOfMaxItems, VARIANT* pItems, ULONG* pNumberOfItemsReturned)
{
	ATLASSERT_POINTER(pItems, VARIANT);
	if(!pItems) {
		return E_POINTER;
	}

	HWND hWndCBox = properties.GetCBoxHWnd();
	ATLASSERT(IsWindow(hWndCBox));

	// check each item in the combo box
	int numberOfItems = static_cast<int>(SendMessage(hWndCBox, CB_GETCOUNT, 0, 0));
	ULONG i = 0;
	for(i = 0; i < numberOfMaxItems; ++i) {
		VariantInit(&pItems[i]);

		do {
			if(properties.lastEnumeratedItem >= 0) {
				if(properties.lastEnumeratedItem < numberOfItems) {
					properties.lastEnumeratedItem = GetNextItemToProcess(properties.lastEnumeratedItem, numberOfItems, hWndCBox);
				}
			} else {
				properties.lastEnumeratedItem = GetFirstItemToProcess(numberOfItems, hWndCBox);
			}
			if(properties.lastEnumeratedItem >= numberOfItems) {
				properties.lastEnumeratedItem = -1;
			}
		} while((properties.lastEnumeratedItem != -1) && (!IsPartOfCollection(properties.lastEnumeratedItem, hWndCBox)));

		if(properties.lastEnumeratedItem != -1) {
			ClassFactory::InitComboItem(properties.lastEnumeratedItem, properties.pOwnerCBox, IID_IDispatch, reinterpret_cast<LPUNKNOWN*>(&pItems[i].pdispVal));
			pItems[i].vt = VT_DISPATCH;
		} else {
			// there's nothing more to iterate
			break;
		}
	}
	if(pNumberOfItemsReturned) {
		*pNumberOfItemsReturned = i;
	}

	return (i == numberOfMaxItems ? S_OK : S_FALSE);
}

STDMETHODIMP ComboBoxItems::Reset(void)
{
	properties.lastEnumeratedItem = -1;
	return S_OK;
}

STDMETHODIMP ComboBoxItems::Skip(ULONG numberOfItemsToSkip)
{
	VARIANT dummy;
	ULONG numItemsReturned = 0;
	// we could skip all items at once, but it's easier to skip them one after the other
	for(ULONG i = 1; i <= numberOfItemsToSkip; ++i) {
		HRESULT hr = Next(1, &dummy, &numItemsReturned);
		VariantClear(&dummy);
		if(hr != S_OK || numItemsReturned == 0) {
			// there're no more items to skip, so don't try it anymore
			break;
		}
	}
	return S_OK;
}
// implementation of IEnumVARIANT
//////////////////////////////////////////////////////////////////////


BOOL ComboBoxItems::IsValidBooleanFilter(VARIANT& filter)
{
	BOOL isValid = TRUE;

	if((filter.vt == (VT_ARRAY | VT_VARIANT)) && filter.parray) {
		LONG lBound = 0;
		LONG uBound = 0;

		if((SafeArrayGetLBound(filter.parray, 1, &lBound) == S_OK) && (SafeArrayGetUBound(filter.parray, 1, &uBound) == S_OK)) {
			// now that we have the bounds, iterate the array
			VARIANT element;
			if(lBound > uBound) {
				isValid = FALSE;
			}
			for(LONG i = lBound; i <= uBound && isValid; ++i) {
				if(SafeArrayGetElement(filter.parray, &i, &element) == S_OK) {
					isValid = (element.vt == VT_BOOL);
					VariantClear(&element);
				} else {
					isValid = FALSE;
				}
			}
		} else {
			isValid = FALSE;
		}
	} else {
		isValid = FALSE;
	}

	return isValid;
}

BOOL ComboBoxItems::IsValidIntegerFilter(VARIANT& filter, int minValue)
{
	BOOL isValid = TRUE;

	if((filter.vt == (VT_ARRAY | VT_VARIANT)) && filter.parray) {
		LONG lBound = 0;
		LONG uBound = 0;

		if((SafeArrayGetLBound(filter.parray, 1, &lBound) == S_OK) && (SafeArrayGetUBound(filter.parray, 1, &uBound) == S_OK)) {
			// now that we have the bounds, iterate the array
			VARIANT element;
			if(lBound > uBound) {
				isValid = FALSE;
			}
			for(LONG i = lBound; i <= uBound && isValid; ++i) {
				if(SafeArrayGetElement(filter.parray, &i, &element) == S_OK) {
					isValid = SUCCEEDED(VariantChangeType(&element, &element, 0, VT_INT)) && element.intVal >= minValue;
					VariantClear(&element);
				} else {
					isValid = FALSE;
				}
			}
		} else {
			isValid = FALSE;
		}
	} else {
		isValid = FALSE;
	}

	return isValid;
}

BOOL ComboBoxItems::IsValidIntegerFilter(VARIANT& filter)
{
	BOOL isValid = TRUE;

	if((filter.vt == (VT_ARRAY | VT_VARIANT)) && filter.parray) {
		LONG lBound = 0;
		LONG uBound = 0;

		if((SafeArrayGetLBound(filter.parray, 1, &lBound) == S_OK) && (SafeArrayGetUBound(filter.parray, 1, &uBound) == S_OK)) {
			// now that we have the bounds, iterate the array
			VARIANT element;
			if(lBound > uBound) {
				isValid = FALSE;
			}
			for(LONG i = lBound; i <= uBound && isValid; ++i) {
				if(SafeArrayGetElement(filter.parray, &i, &element) == S_OK) {
					isValid = SUCCEEDED(VariantChangeType(&element, &element, 0, VT_UI4));
					VariantClear(&element);
				} else {
					isValid = FALSE;
				}
			}
		} else {
			isValid = FALSE;
		}
	} else {
		isValid = FALSE;
	}

	return isValid;
}

BOOL ComboBoxItems::IsValidStringFilter(VARIANT& filter)
{
	BOOL isValid = TRUE;

	if((filter.vt == (VT_ARRAY | VT_VARIANT)) && filter.parray) {
		LONG lBound = 0;
		LONG uBound = 0;

		if((SafeArrayGetLBound(filter.parray, 1, &lBound) == S_OK) && (SafeArrayGetUBound(filter.parray, 1, &uBound) == S_OK)) {
			// now that we have the bounds, iterate the array
			VARIANT element;
			if(lBound > uBound) {
				isValid = FALSE;
			}
			for(LONG i = lBound; i <= uBound && isValid; ++i) {
				if(SafeArrayGetElement(filter.parray, &i, &element) == S_OK) {
					isValid = (element.vt == VT_BSTR);
					VariantClear(&element);
				} else {
					isValid = FALSE;
				}
			}
		} else {
			isValid = FALSE;
		}
	} else {
		isValid = FALSE;
	}

	return isValid;
}

int ComboBoxItems::GetFirstItemToProcess(int numberOfItems, HWND /*hWndCBox*/)
{
	//ATLASSERT(IsWindow(hWndCBox));

	if(numberOfItems == 0) {
		return -1;
	}
	return 0;
}

int ComboBoxItems::GetNextItemToProcess(int previousItem, int numberOfItems, HWND /*hWndCBox*/)
{
	//ATLASSERT(IsWindow(hWndCBox));

	if(previousItem < numberOfItems - 1) {
		return previousItem + 1;
	}
	return -1;
}

BOOL ComboBoxItems::IsBooleanInSafeArray(LPSAFEARRAY pSafeArray, VARIANT_BOOL value, LPVOID pComparisonFunction)
{
	LONG lBound = 0;
	LONG uBound = 0;
	SafeArrayGetLBound(pSafeArray, 1, &lBound);
	SafeArrayGetUBound(pSafeArray, 1, &uBound);

	VARIANT element;
	BOOL foundMatch = FALSE;
	for(LONG i = lBound; i <= uBound && !foundMatch; ++i) {
		SafeArrayGetElement(pSafeArray, &i, &element);
		if(pComparisonFunction) {
			typedef BOOL WINAPI ComparisonFn(VARIANT_BOOL, VARIANT_BOOL);
			ComparisonFn* pComparisonFn = reinterpret_cast<ComparisonFn*>(pComparisonFunction);
			if(pComparisonFn(value, element.boolVal)) {
				foundMatch = TRUE;
			}
		} else {
			if(element.boolVal == value) {
				foundMatch = TRUE;
			}
		}
		VariantClear(&element);
	}

	return foundMatch;
}

BOOL ComboBoxItems::IsIntegerInSafeArray(LPSAFEARRAY pSafeArray, int value, LPVOID pComparisonFunction)
{
	LONG lBound = 0;
	LONG uBound = 0;
	SafeArrayGetLBound(pSafeArray, 1, &lBound);
	SafeArrayGetUBound(pSafeArray, 1, &uBound);

	VARIANT element;
	BOOL foundMatch = FALSE;
	for(LONG i = lBound; i <= uBound && !foundMatch; ++i) {
		SafeArrayGetElement(pSafeArray, &i, &element);
		int v = 0;
		if(SUCCEEDED(VariantChangeType(&element, &element, 0, VT_INT))) {
			v = element.intVal;
		}
		if(pComparisonFunction) {
			typedef BOOL WINAPI ComparisonFn(LONG, LONG);
			ComparisonFn* pComparisonFn = reinterpret_cast<ComparisonFn*>(pComparisonFunction);
			if(pComparisonFn(value, v)) {
				foundMatch = TRUE;
			}
		} else {
			if(v == value) {
				foundMatch = TRUE;
			}
		}
		VariantClear(&element);
	}

	return foundMatch;
}

BOOL ComboBoxItems::IsStringInSafeArray(LPSAFEARRAY pSafeArray, BSTR value, LPVOID pComparisonFunction)
{
	LONG lBound = 0;
	LONG uBound = 0;
	SafeArrayGetLBound(pSafeArray, 1, &lBound);
	SafeArrayGetUBound(pSafeArray, 1, &uBound);

	VARIANT element;
	BOOL foundMatch = FALSE;
	for(LONG i = lBound; i <= uBound && !foundMatch; ++i) {
		SafeArrayGetElement(pSafeArray, &i, &element);
		if(pComparisonFunction) {
			typedef BOOL WINAPI ComparisonFn(BSTR, BSTR);
			ComparisonFn* pComparisonFn = reinterpret_cast<ComparisonFn*>(pComparisonFunction);
			if(pComparisonFn(value, element.bstrVal)) {
				foundMatch = TRUE;
			}
		} else {
			if(properties.caseSensitiveFilters) {
				if(lstrcmpW(OLE2W(element.bstrVal), OLE2W(value)) == 0) {
					foundMatch = TRUE;
				}
			} else {
				if(lstrcmpiW(OLE2W(element.bstrVal), OLE2W(value)) == 0) {
					foundMatch = TRUE;
				}
			}
		}
		VariantClear(&element);
	}

	return foundMatch;
}

BOOL ComboBoxItems::IsPartOfCollection(int itemIndex, HWND hWndCBox/* = NULL*/)
{
	if(!hWndCBox) {
		hWndCBox = properties.GetCBoxHWnd();
	}
	ATLASSERT(IsWindow(hWndCBox));

	if(!IsValidComboBoxItemIndex(itemIndex, hWndCBox)) {
		return FALSE;
	}

	BOOL isPart = FALSE;

	if(properties.filterType[fpIndex] != ftDeactivated) {
		if(IsIntegerInSafeArray(properties.filter[fpIndex].parray, itemIndex, properties.comparisonFunction[fpIndex])) {
			if(properties.filterType[fpIndex] == ftExcluding) {
				goto Exit;
			}
		} else {
			if(properties.filterType[fpIndex] == ftIncluding) {
				goto Exit;
			}
		}
	}
	if(properties.filterType[fpItemData] != ftDeactivated) {
		if(IsIntegerInSafeArray(properties.filter[fpItemData].parray, static_cast<int>(SendMessage(hWndCBox, CB_GETITEMDATA, itemIndex, 0)), properties.comparisonFunction[fpItemData])) {
			if(properties.filterType[fpItemData] == ftExcluding) {
				goto Exit;
			}
		} else {
			if(properties.filterType[fpItemData] == ftIncluding) {
				goto Exit;
			}
		}
	}
	if(properties.filterType[fpText] != ftDeactivated) {
		CComBSTR text = L"";

		BOOL expectsString;
		DWORD style = CWindow(hWndCBox).GetStyle();
		if(style & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE)) {
			expectsString = ((style & CBS_HASSTRINGS) == CBS_HASSTRINGS);
		} else {
			expectsString = TRUE;
		}
		if(expectsString) {
			int textLength = static_cast<int>(SendMessage(hWndCBox, CB_GETLBTEXTLEN, itemIndex, 0));
			if(textLength != CB_ERR) {
				LPTSTR pBuffer = static_cast<LPTSTR>(HeapAlloc(GetProcessHeap(), 0, (textLength + 1) * sizeof(TCHAR)));
				if(pBuffer) {
					ZeroMemory(pBuffer, (textLength + 1) * sizeof(TCHAR));
					SendMessage(hWndCBox, CB_GETLBTEXT, itemIndex, reinterpret_cast<LPARAM>(pBuffer));
					text = pBuffer;
					HeapFree(GetProcessHeap(), 0, pBuffer);
				}
			}
		}

		if(IsStringInSafeArray(properties.filter[fpText].parray, text, properties.comparisonFunction[fpText])) {
			if(properties.filterType[fpText] == ftExcluding) {
				goto Exit;
			}
		} else {
			if(properties.filterType[fpText] == ftIncluding) {
				goto Exit;
			}
		}
	}
	isPart = TRUE;

Exit:
	return isPart;
}

void ComboBoxItems::OptimizeFilter(FilteredPropertyConstants filteredProperty)
{
	if(filteredProperty != fpSelected) {
		// currently we optimize boolean filters only
		return;
	}

	LONG lBound = 0;
	LONG uBound = 0;
	SafeArrayGetLBound(properties.filter[filteredProperty].parray, 1, &lBound);
	SafeArrayGetUBound(properties.filter[filteredProperty].parray, 1, &uBound);

	// now that we have the bounds, iterate the array
	VARIANT element;
	UINT numberOfTrues = 0;
	UINT numberOfFalses = 0;
	for(LONG i = lBound; i <= uBound; ++i) {
		SafeArrayGetElement(properties.filter[filteredProperty].parray, &i, &element);
		if(element.boolVal == VARIANT_FALSE) {
			++numberOfFalses;
		} else {
			++numberOfTrues;
		}

		VariantClear(&element);
	}

	if(numberOfTrues > 0 && numberOfFalses > 0) {
		// we've something like True Or False Or True - we can deactivate this filter
		properties.filterType[filteredProperty] = ftDeactivated;
		VariantClear(&properties.filter[filteredProperty]);
	} else if(numberOfTrues == 0 && numberOfFalses > 1) {
		// False Or False Or False... is still False, so we need just one False
		VariantClear(&properties.filter[filteredProperty]);
		properties.filter[filteredProperty].vt = VT_ARRAY | VT_VARIANT;
		properties.filter[filteredProperty].parray = SafeArrayCreateVectorEx(VT_VARIANT, 1, 1, NULL);

		VARIANT newElement;
		VariantInit(&newElement);
		newElement.vt = VT_BOOL;
		newElement.boolVal = VARIANT_FALSE;
		LONG index = 1;
		SafeArrayPutElement(properties.filter[filteredProperty].parray, &index, &newElement);
		VariantClear(&newElement);
	} else if(numberOfFalses == 0 && numberOfTrues > 1) {
		// True Or True Or True... is still True, so we need just one True
		VariantClear(&properties.filter[filteredProperty]);
		properties.filter[filteredProperty].vt = VT_ARRAY | VT_VARIANT;
		properties.filter[filteredProperty].parray = SafeArrayCreateVectorEx(VT_VARIANT, 1, 1, NULL);

		VARIANT newElement;
		VariantInit(&newElement);
		newElement.vt = VT_BOOL;
		newElement.boolVal = VARIANT_TRUE;
		LONG index = 1;
		SafeArrayPutElement(properties.filter[filteredProperty].parray, &index, &newElement);
		VariantClear(&newElement);
	}
}

#ifdef USE_STL
	HRESULT ComboBoxItems::RemoveItems(std::list<int>& itemsToRemove, HWND hWndCBox)
#else
	HRESULT ComboBoxItems::RemoveItems(CAtlList<int>& itemsToRemove, HWND hWndCBox)
#endif
{
	ATLASSERT(IsWindow(hWndCBox));

	CWindowEx2(hWndCBox).InternalSetRedraw(FALSE);
	// sort in reverse order
	#ifdef USE_STL
		itemsToRemove.sort(std::greater<int>());
		for(std::list<int>::const_iterator iter = itemsToRemove.begin(); iter != itemsToRemove.end(); ++iter) {
			SendMessage(hWndCBox, CB_DELETESTRING, *iter, 0);
		}
	#else
		// perform a crude bubble sort
		for(size_t j = 0; j < itemsToRemove.GetCount(); ++j) {
			for(size_t i = 0; i < itemsToRemove.GetCount() - 1; ++i) {
				if(itemsToRemove.GetAt(itemsToRemove.FindIndex(i)) < itemsToRemove.GetAt(itemsToRemove.FindIndex(i + 1))) {
					itemsToRemove.SwapElements(itemsToRemove.FindIndex(i), itemsToRemove.FindIndex(i + 1));
				}
			}
		}

		for(size_t i = 0; i < itemsToRemove.GetCount(); ++i) {
			SendMessage(hWndCBox, CB_DELETESTRING, itemsToRemove.GetAt(itemsToRemove.FindIndex(i)), 0);
		}
	#endif
	CWindowEx2(hWndCBox).InternalSetRedraw(TRUE);

	return S_OK;
}


ComboBoxItems::Properties::~Properties()
{
	for(int i = 0; i < NUMBEROFFILTERS; ++i) {
		VariantClear(&filter[i]);
	}
	if(pOwnerCBox) {
		pOwnerCBox->Release();
	}
}

void ComboBoxItems::Properties::CopyTo(ComboBoxItems::Properties* pTarget)
{
	ATLASSERT_POINTER(pTarget, Properties);
	if(pTarget) {
		pTarget->pOwnerCBox = this->pOwnerCBox;
		if(pTarget->pOwnerCBox) {
			pTarget->pOwnerCBox->AddRef();
		}
		pTarget->lastEnumeratedItem = this->lastEnumeratedItem;
		pTarget->caseSensitiveFilters = this->caseSensitiveFilters;

		for(int i = 0; i < NUMBEROFFILTERS; ++i) {
			VariantCopy(&pTarget->filter[i], &this->filter[i]);
			pTarget->filterType[i] = this->filterType[i];
			pTarget->comparisonFunction[i] = this->comparisonFunction[i];
		}
		pTarget->usingFilters = this->usingFilters;
	}
}

HWND ComboBoxItems::Properties::GetCBoxHWnd(void)
{
	ATLASSUME(pOwnerCBox);

	OLE_HANDLE handle = NULL;
	pOwnerCBox->get_hWnd(&handle);
	return static_cast<HWND>(LongToHandle(handle));
}


void ComboBoxItems::SetOwner(ComboBox* pOwner)
{
	if(properties.pOwnerCBox) {
		properties.pOwnerCBox->Release();
	}
	properties.pOwnerCBox = pOwner;
	if(properties.pOwnerCBox) {
		properties.pOwnerCBox->AddRef();
	}
}


STDMETHODIMP ComboBoxItems::get_CaseSensitiveFilters(VARIANT_BOOL* pValue)
{
	ATLASSERT_POINTER(pValue, VARIANT_BOOL);
	if(!pValue) {
		return E_POINTER;
	}

	*pValue = BOOL2VARIANTBOOL(properties.caseSensitiveFilters);
	return S_OK;
}

STDMETHODIMP ComboBoxItems::put_CaseSensitiveFilters(VARIANT_BOOL newValue)
{
	properties.caseSensitiveFilters = VARIANTBOOL2BOOL(newValue);
	return S_OK;
}

STDMETHODIMP ComboBoxItems::get_ComparisonFunction(FilteredPropertyConstants filteredProperty, LONG* pValue)
{
	ATLASSERT_POINTER(pValue, LONG);
	if(!pValue) {
		return E_POINTER;
	}

	switch(filteredProperty) {
		case fpIndex:
		case fpItemData:
		case fpText:
			*pValue = static_cast<LONG>(reinterpret_cast<LONG_PTR>(properties.comparisonFunction[filteredProperty]));
			return S_OK;
			break;
	}
	return E_INVALIDARG;
}

STDMETHODIMP ComboBoxItems::put_ComparisonFunction(FilteredPropertyConstants filteredProperty, LONG newValue)
{
	switch(filteredProperty) {
		case fpIndex:
		case fpItemData:
		case fpText:
			properties.comparisonFunction[filteredProperty] = reinterpret_cast<LPVOID>(static_cast<LONG_PTR>(newValue));
			return S_OK;
			break;
	}
	return E_INVALIDARG;
}

STDMETHODIMP ComboBoxItems::get_Filter(FilteredPropertyConstants filteredProperty, VARIANT* pValue)
{
	ATLASSERT_POINTER(pValue, VARIANT);
	if(!pValue) {
		return E_POINTER;
	}

	switch(filteredProperty) {
		case fpIndex:
		case fpItemData:
		case fpText:
			VariantClear(pValue);
			VariantCopy(pValue, &properties.filter[filteredProperty]);
			return S_OK;
			break;
	}
	return E_INVALIDARG;
}

STDMETHODIMP ComboBoxItems::put_Filter(FilteredPropertyConstants filteredProperty, VARIANT newValue)
{
	// check 'newValue'
	switch(filteredProperty) {
		case fpIndex:
			if(!IsValidIntegerFilter(newValue, 0)) {
				// invalid value - raise VB runtime error 380
				return MAKE_HRESULT(1, FACILITY_WINDOWS | FACILITY_DISPATCH, 380);
			}
			break;
		case fpItemData:
			if(!IsValidIntegerFilter(newValue)) {
				// invalid value - raise VB runtime error 380
				return MAKE_HRESULT(1, FACILITY_WINDOWS | FACILITY_DISPATCH, 380);
			}
			break;
		case fpText:
			if(!IsValidStringFilter(newValue)) {
				// invalid value - raise VB runtime error 380
				return MAKE_HRESULT(1, FACILITY_WINDOWS | FACILITY_DISPATCH, 380);
			}
			break;
		default:
			return E_INVALIDARG;
			break;
	}

	VariantClear(&properties.filter[filteredProperty]);
	VariantCopy(&properties.filter[filteredProperty], &newValue);
	OptimizeFilter(filteredProperty);
	return S_OK;
}

STDMETHODIMP ComboBoxItems::get_FilterType(FilteredPropertyConstants filteredProperty, FilterTypeConstants* pValue)
{
	ATLASSERT_POINTER(pValue, FilterTypeConstants);
	if(!pValue) {
		return E_POINTER;
	}

	switch(filteredProperty) {
		case fpIndex:
		case fpItemData:
		case fpText:
			*pValue = properties.filterType[filteredProperty];
			return S_OK;
			break;
	}
	return E_INVALIDARG;
}

STDMETHODIMP ComboBoxItems::put_FilterType(FilteredPropertyConstants filteredProperty, FilterTypeConstants newValue)
{
	if(newValue < 0 || newValue > 2) {
		// invalid value - raise VB runtime error 380
		return MAKE_HRESULT(1, FACILITY_WINDOWS | FACILITY_DISPATCH, 380);
	}

	switch(filteredProperty) {
		case fpIndex:
		case fpItemData:
		case fpText:
			properties.filterType[filteredProperty] = newValue;
			if(newValue != ftDeactivated) {
				properties.usingFilters = TRUE;
			} else {
				properties.usingFilters = FALSE;
				for(int i = 0; i < NUMBEROFFILTERS; ++i) {
					if(properties.filterType[i] != ftDeactivated) {
						properties.usingFilters = TRUE;
						break;
					}
				}
			}
			return S_OK;
			break;
	}
	return E_INVALIDARG;
}

STDMETHODIMP ComboBoxItems::get_Item(LONG itemIdentifier, ItemIdentifierTypeConstants itemIdentifierType/* = iitIndex*/, IComboBoxItem** ppItem/* = NULL*/)
{
	ATLASSERT_POINTER(ppItem, IComboBoxItem*);
	if(!ppItem) {
		return E_POINTER;
	}

	// retrieve the item's index
	int itemIndex = -2;
	switch(itemIdentifierType) {
		case iitID:
			if(properties.pOwnerCBox) {
				itemIndex = properties.pOwnerCBox->IDToItemIndex(itemIdentifier);
			}
			break;
		case iitIndex:
			itemIndex = itemIdentifier;
			break;
	}

	if(itemIndex > -1) {
		if(IsPartOfCollection(itemIndex)) {
			ClassFactory::InitComboItem(itemIndex, properties.pOwnerCBox, IID_IComboBoxItem, reinterpret_cast<LPUNKNOWN*>(ppItem));
		}
		return S_OK;
	} else {
		// item not found
		if(itemIdentifierType == iitIndex) {
			return DISP_E_BADINDEX;
		} else {
			return E_INVALIDARG;
		}
	}
}

STDMETHODIMP ComboBoxItems::get__NewEnum(IUnknown** ppEnumerator)
{
	ATLASSERT_POINTER(ppEnumerator, LPUNKNOWN);
	if(!ppEnumerator) {
		return E_POINTER;
	}

	Reset();
	return QueryInterface(IID_IUnknown, reinterpret_cast<LPVOID*>(ppEnumerator));
}


STDMETHODIMP ComboBoxItems::Add(BSTR itemText, LONG insertAt/* = -1*/, LONG itemData/* = 0*/, IComboBoxItem** ppAddedItem/* = NULL*/)
{
	ATLASSERT_POINTER(ppAddedItem, IComboBoxItem*);
	if(!ppAddedItem) {
		return E_POINTER;
	}

	if(insertAt < -1) {
		// invalid value - raise VB runtime error 380
		return MAKE_HRESULT(1, FACILITY_WINDOWS | FACILITY_DISPATCH, 380);
	}

	HWND hWndCBox = properties.GetCBoxHWnd();
	ATLASSERT(IsWindow(hWndCBox));
	BOOL expectsString;
	DWORD style = CWindow(hWndCBox).GetStyle();
	if(style & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE)) {
		expectsString = ((style & CBS_HASSTRINGS) == CBS_HASSTRINGS);
	} else {
		expectsString = TRUE;
	}

	COLE2T converter(itemText);
	LPTSTR pBuffer = converter;

	int itemIndex = -1;
	if(insertAt == -1) {
		// we could use CB_INSERTSTRING as well, but this wouldn't cause a sorted list to sort
		if(expectsString) {
			properties.pOwnerCBox->BufferItemData(itemData);
			itemIndex = static_cast<int>(SendMessage(hWndCBox, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pBuffer)));
		} else {
			itemIndex = static_cast<int>(SendMessage(hWndCBox, CB_ADDSTRING, 0, itemData));
		}
	} else {
		if(expectsString) {
			properties.pOwnerCBox->BufferItemData(itemData);
			itemIndex = static_cast<int>(SendMessage(hWndCBox, CB_INSERTSTRING, insertAt, reinterpret_cast<LPARAM>(pBuffer)));
		} else {
			itemIndex = static_cast<int>(SendMessage(hWndCBox, CB_INSERTSTRING, insertAt, itemData));
		}
	}
	if(itemIndex > CB_ERR) {
		ClassFactory::InitComboItem(itemIndex, properties.pOwnerCBox, IID_IComboBoxItem, reinterpret_cast<LPUNKNOWN*>(ppAddedItem));
		return S_OK;
	}
	*ppAddedItem = NULL;
	return E_FAIL;
}

STDMETHODIMP ComboBoxItems::Contains(LONG itemIdentifier, ItemIdentifierTypeConstants itemIdentifierType/* = iitIndex*/, VARIANT_BOOL* pValue/* = NULL*/)
{
	ATLASSERT_POINTER(pValue, VARIANT_BOOL);
	if(!pValue) {
		return E_POINTER;
	}

	// retrieve the item's index
	int itemIndex = -2;
	switch(itemIdentifierType) {
		case iitID:
			if(properties.pOwnerCBox) {
				itemIndex = properties.pOwnerCBox->IDToItemIndex(itemIdentifier);
			}
			break;
		case iitIndex:
			itemIndex = itemIdentifier;
			break;
	}

	*pValue = BOOL2VARIANTBOOL(IsPartOfCollection(itemIndex));
	return S_OK;
}

STDMETHODIMP ComboBoxItems::Count(LONG* pValue)
{
	ATLASSERT_POINTER(pValue, LONG);
	if(!pValue) {
		return E_POINTER;
	}

	HWND hWndCBox = properties.GetCBoxHWnd();
	ATLASSERT(IsWindow(hWndCBox));

	if(!properties.usingFilters) {
		*pValue = static_cast<LONG>(SendMessage(hWndCBox, CB_GETCOUNT, 0, 0));
	}

	// count the items "by hand"
	*pValue = 0;
	int numberOfItems = static_cast<int>(SendMessage(hWndCBox, CB_GETCOUNT, 0, 0));
	int itemIndex = GetFirstItemToProcess(numberOfItems, hWndCBox);
	while(itemIndex != -1) {
		if(IsPartOfCollection(itemIndex, hWndCBox)) {
			++(*pValue);
		}
		itemIndex = GetNextItemToProcess(itemIndex, numberOfItems, hWndCBox);
	}
	return S_OK;
}

STDMETHODIMP ComboBoxItems::Remove(LONG itemIdentifier, ItemIdentifierTypeConstants itemIdentifierType/* = iitIndex*/)
{
	HWND hWndCBox = properties.GetCBoxHWnd();
	ATLASSERT(IsWindow(hWndCBox));

	// retrieve the item's index
	int itemIndex = -2;
	switch(itemIdentifierType) {
		case iitID:
			if(properties.pOwnerCBox) {
				itemIndex = properties.pOwnerCBox->IDToItemIndex(itemIdentifier);
			}
			break;
		case iitIndex:
			itemIndex = itemIdentifier;
			break;
	}

	if(itemIndex != -1) {
		if(IsPartOfCollection(itemIndex)) {
			if(SendMessage(hWndCBox, CB_DELETESTRING, itemIndex, 0) != CB_ERR) {
				return S_OK;
			}
		}
	} else {
		// item not found
		if(itemIdentifierType == iitIndex) {
			return DISP_E_BADINDEX;
		} else {
			return E_INVALIDARG;
		}
	}

	return E_FAIL;
}

STDMETHODIMP ComboBoxItems::RemoveAll(void)
{
	HWND hWndCBox = properties.GetCBoxHWnd();
	ATLASSERT(IsWindow(hWndCBox));

	if(!properties.usingFilters) {
		SendMessage(hWndCBox, CB_RESETCONTENT, 0, 0);
		return S_OK;
	}

	// find the items to remove manually
	#ifdef USE_STL
		std::list<int> itemsToRemove;
	#else
		CAtlList<int> itemsToRemove;
	#endif
	int numberOfItems = static_cast<int>(SendMessage(hWndCBox, CB_GETCOUNT, 0, 0));
	int itemIndex = GetFirstItemToProcess(numberOfItems, hWndCBox);
	while(itemIndex != -1) {
		if(IsPartOfCollection(itemIndex, hWndCBox)) {
			#ifdef USE_STL
				itemsToRemove.push_back(itemIndex);
			#else
				itemsToRemove.AddTail(itemIndex);
			#endif
		}
		itemIndex = GetNextItemToProcess(itemIndex, numberOfItems, hWndCBox);
	}
	return RemoveItems(itemsToRemove, hWndCBox);
}