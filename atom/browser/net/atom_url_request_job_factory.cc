// Copyright (c) 2013 GitHub, Inc. All rights reserved.
// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/net/atom_url_request_job_factory.h"

#include "base/stl_util.h"
#include "net/base/load_flags.h"
#include "net/url_request/url_request.h"

namespace atom {

typedef net::URLRequestJobFactory::ProtocolHandler ProtocolHandler;

AtomURLRequestJobFactory::AtomURLRequestJobFactory() {}

AtomURLRequestJobFactory::~AtomURLRequestJobFactory() {
  STLDeleteValues(&protocol_handler_map_);
}

bool AtomURLRequestJobFactory::SetProtocolHandler(
    const std::string& scheme,
    ProtocolHandler* protocol_handler) {
  DCHECK(CalledOnValidThread());

  base::AutoLock locked(lock_);

  if (!protocol_handler) {
    ProtocolHandlerMap::iterator it = protocol_handler_map_.find(scheme);
    if (it == protocol_handler_map_.end())
      return false;

    delete it->second;
    protocol_handler_map_.erase(it);
    return true;
  }

  if (ContainsKey(protocol_handler_map_, scheme))
    return false;
  protocol_handler_map_[scheme] = protocol_handler;
  return true;
}

ProtocolHandler* AtomURLRequestJobFactory::ReplaceProtocol(
    const std::string& scheme,
    ProtocolHandler* protocol_handler) {
  DCHECK(CalledOnValidThread());
  DCHECK(protocol_handler);

  base::AutoLock locked(lock_);
  if (!ContainsKey(protocol_handler_map_, scheme))
    return NULL;
  ProtocolHandler* original_protocol_handler = protocol_handler_map_[scheme];
  protocol_handler_map_[scheme] = protocol_handler;
  return original_protocol_handler;
}

ProtocolHandler* AtomURLRequestJobFactory::GetProtocolHandler(
    const std::string& scheme) const {
  DCHECK(CalledOnValidThread());

  base::AutoLock locked(lock_);
  ProtocolHandlerMap::const_iterator it = protocol_handler_map_.find(scheme);
  if (it == protocol_handler_map_.end())
    return NULL;
  return it->second;
}

bool AtomURLRequestJobFactory::HasProtocolHandler(
    const std::string& scheme) const {
  base::AutoLock locked(lock_);
  return ContainsKey(protocol_handler_map_, scheme);
}

net::URLRequestJob* AtomURLRequestJobFactory::MaybeCreateJobWithProtocolHandler(
    const std::string& scheme,
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate) const {
  DCHECK(CalledOnValidThread());

  base::AutoLock locked(lock_);
  ProtocolHandlerMap::const_iterator it = protocol_handler_map_.find(scheme);
  if (it == protocol_handler_map_.end())
    return NULL;
  return it->second->MaybeCreateJob(request, network_delegate);
}

bool AtomURLRequestJobFactory::IsHandledProtocol(
    const std::string& scheme) const {
  DCHECK(CalledOnValidThread());
  return HasProtocolHandler(scheme) ||
      net::URLRequest::IsHandledProtocol(scheme);
}

bool AtomURLRequestJobFactory::IsHandledURL(const GURL& url) const {
  if (!url.is_valid()) {
    // We handle error cases.
    return true;
  }
  return IsHandledProtocol(url.scheme());
}

bool AtomURLRequestJobFactory::IsSafeRedirectTarget(
    const GURL& location) const {
  return IsHandledURL(location);
}

}  // namespace atom
