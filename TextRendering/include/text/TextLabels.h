/*
 Copyright (c) 2010-2012, Paul Houx - All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "cinder/DataSource.h"
#include "cinder/TriMesh.h"
#include "cinder/Utilities.h"
#include "cinder/gl/Vbo.h"
#include "text/Text.h"

namespace ph { namespace text {

class TextLabelCompare
{
public:
	TextLabelCompare() {};

	// always append at the end
	bool operator() (const ci::Vec3f &a, const ci::Vec3f &b) const {
        return false;
    }
};

typedef std::multimap< ci::Vec3f, std::wstring, TextLabelCompare >	TextLabelList;
typedef TextLabelList::iterator			TextLabelListIter;
typedef TextLabelList::const_iterator	TextLabelListConstIter;
	
class TextLabels
	: public ph::text::Text
{
public:
	TextLabels(void)
		: mOffset( ci::Vec2f::zero() ) {};
	virtual ~TextLabels(void) {};

	//! clears all labels
	void	clear();
	//! returns the number of labels 
	size_t	size() const { return mLabels.size(); }
	//! returns a const iterator to the labels
	TextLabelListConstIter	begin() const { return mLabels.begin(); }
	//! returns a const iterator to the labels
	TextLabelListConstIter	end() const { return mLabels.end(); }

	//!
	//ci::Vec2f	getOffset() const { return mOffset; }
	//void		setOffset( float x, float y ) { setOffset( ci::Vec2f(x, y) ); }
	//void		setOffset( const ci::Vec2f &offset ) { mOffset = offset; mInvalid = true; }

	//!	add label
	void	addLabel( const ci::Vec3f &position, const std::string &text ) { addLabel(position, ci::toUtf16(text)); }
	void	addLabel( const ci::Vec3f &position, const std::wstring &text );
protected:
	//! get the maximum width of the text at the specified vertical position
	virtual float getWidthAt(float y) const { return 1000.0f; }
	
	//! override vertex shader and bind method
	virtual std::string	getVertexShader() const;
	virtual bool		bindShader();
	
	//! clears the mesh and the buffers
	virtual void		clearMesh();
	//! renders the current contents of mText
	virtual void		renderMesh();
	//! helper to render a non-word-wrapped string
	virtual void		renderString( const std::wstring &str, ci::Vec2f *cursor, float stretch=1.0f );
	//! creates the VBO from the data in the buffers
	virtual void		createMesh();
private:
	TextLabelList			mLabels;
	
	ci::Vec3f				mOffset;
	std::vector<ci::Vec3f>	mOffsets;
};

} } // namespace ph::text