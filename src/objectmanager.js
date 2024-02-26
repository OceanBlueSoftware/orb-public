/**
 * @fileOverview Monitors the document and certain JavaScript mechanisms for objects. Builds or
 * upgrades those objects if there is a registered handler.
 * @license ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

hbbtv.objects = {};

hbbtv.objectManager = (function() {
    const __createElement = document.createElement;

    let objectMimeTypeTable = [];
    let objectFactoryMethodTable = [];
    let objectUpgradeHandlers = [];
    let imageFactor = 1;
    let defaultDevicePixelRatio = -1;
    let graphicPlane = 720;

    function initialise() {
        addMetaViewportElement();
        addOipfObjectFactory();
        addHTMLManipulationIntercept(function(manipulatedElement) {
            upgradeDescendantObjects(manipulatedElement);
        });
        addCreateElementTypeIntercept(function(object, type) {
            upgradeObject(object, type);
        });
        addMutationIntercept(
            function(addedObject) {
                hbbtv.holePuncher.notifyObjectAddedToDocument(addedObject);
                upgradeObject(addedObject, addedObject.getAttribute('type'));
            },
            function(removedObject) {
                hbbtv.holePuncher.notifyObjectRemovedFromDocument(removedObject);
            }
        );
        upgradeDescendantObjects(document);
        defaultDevicePixelRatio = window.devicePixelRatio;
        graphicPlane = hbbtv.bridge.configuration.getRenderingResolution();
        check();
    }

    function check() {
        Object.defineProperty(window, 'devicePixelRatio', {
            value: defaultDevicePixelRatio * (graphicPlane / 720.0),
            writable: false
        });
    }

    function registerObject(options) {
        //console.debug("Register object " + options.name + " with mime types " + options.mimeTypes);
        for (const mimeType of options.mimeTypes) {
            objectMimeTypeTable[mimeType] = options.name;
        }
        if (options.oipfObjectFactoryMethodName.length > 0) {
            objectFactoryMethodTable[options.oipfObjectFactoryMethodName] = options.name;
        }
        objectUpgradeHandlers[options.name] = options.upgradeObject;
    }

    function resolveMimeType(mimeType) {
        if (mimeType in objectMimeTypeTable) {
            return objectMimeTypeTable[mimeType];
        }
        console.debug('Could not resolve mime type ' + mimeType);
        return undefined;
    }

    function upgradeDescendantObjects(target) {
        for (const object of target.getElementsByTagName('object')) {
            if (object.hasAttribute('type')) {
                upgradeObject(object, object.getAttribute('type'));
            }
        }
    }

    function upgradeObject(object, mimeType) {
        if (!mimeType) {
            return;
        }
        mimeType = mimeType.toLowerCase();
        if (object.hasAttribute('__mimeType') && object.getAttribute('__mimeType') === mimeType) {
            // Already done
            return;
        }
        const objectName = resolveMimeType(mimeType);
        if (objectName !== undefined) {
            let mimeTypeChanged = false;
            if (object.hasAttribute('__objectType')) {
                if (object.getAttribute('__objectType') !== objectName) {
                    // Ignore changes to the object type (but not the MIME type) [OIPF 5 4.4.4]
                    return;
                }
                mimeTypeChanged = true;
            }
            object.setAttribute('__mimeType', mimeType);
            object.setAttribute('__objectType', objectName);
            if (mimeTypeChanged) {
                // TODO
            } else {
                objectUpgradeHandlers[objectName](object);
            }
        }
    }

    function updateImageSrc(imageElement) {
        if (imageElement.id == 'cimg') {
            console.log("(imageElement.id == 'cimg')");
            return ;
        }
        let srcsetValue = imageElement.getAttribute('srcset');
        let sizesValue = imageElement.getAttribute('sizes');
        let descriptor = (sizesValue == null) ? 'x' : 'w';
        console.log(srcsetValue);
        if (srcsetValue != null) {
            let sources = srcsetValue.split(',');
            let newSrcSet = '';
            sources.forEach(function(source) {
                source = source.trim();
                let parts = source.split(' ');
                let widthDescriptor = parseFloat(parts[1].match(/\d+(\.\d+)?/));
                console.log(widthDescriptor);
                if (newSrcSet !== '') {
                    newSrcSet += ', ';
                }
                newSrcSet += parts[0] + ' ' + widthDescriptor * imageFactor + descriptor;
            })
            console.log(newSrcSet); 
            imageElement.setAttribute('srcset', newSrcSet);

            // if (sizesValue == null) {
            //     console.log("checkkkkkkkkkkkkkkkkkkk");
            //     // var percentage = (imageFactor * 100) + '%';
            //     // var style = 'width: ' + percentage + '; height: auto; ';
            //     var style = 'width: 60%; height: auto; ';
            //     imageElement.setAttribute('style', style);
            //     console.log("checkkkkkkkkkkkkkkkkkkk");
            //     console.log(imageElement.getAttribute('style'));
            // }
        }
    }

    // Object.defineProperty(window, 'devicePixelRatio', {
    //     value: defaultDevicePixelRatio * (graphicPlane / 720.0),
    //     writable: false
    // });

    // Mutation observer
    function addMutationIntercept(callbackObjectAdded, callbackObjectRemoved) {
        const observer = new MutationObserver(function(mutationsList) {
            console.log("defaultDevicePixelRatio");
            console.log(defaultDevicePixelRatio);
            console.log("window.devicePixelRatio");
            console.log(window.devicePixelRatio);
            imageFactor = 720.0 / graphicPlane;
            console.log("imageFactor");
            console.log(imageFactor);
            console.log("window.innerHeight");
            console.log(window.innerHeight);
            console.log("document.documentElement.clientHeight");
            console.log(document.documentElement.clientHeight);
            for (const mutation of mutationsList) {
                if (mutation.type === 'childList') {
                    for (const node of mutation.addedNodes) {
                        // var dpr = window.devicePixelRatio || 1;
                        // console.log("pixelRatio : " + dpr);
                        if (node.nodeType === Node.ELEMENT_NODE && node.nodeName.toLowerCase() === 'img') {
                            // console.log("updateImageSrc");
                            // var dpr = window.devicePixelRatio || 1;
                            // console.log("pixelRatio : " + dpr);
                            // console.log("imageFactor : " + imageFactor);
                            updateImageSrc(node);
                        }
                        // if (node.srcset != null) {
                        //     console.log("removeAttribute");
                        //     console.log(node.id);
                        //     var dpr = window.devicePixelRatio || 1;
                        //     console.log(dpr);
                        //     node.removeAttribute('srcset');

                        // }
                        
                        // console.log(node);
                        // console.log(node.src);
                        // console.log(node.srcset);
                        // console.log(node.style);
                        if (node.nodeName && node.nodeName.toLowerCase() === 'object') {
                            callbackObjectAdded(node);
                        }
                    }
                    for (const node of mutation.removedNodes) {
                        if (node.nodeName && node.nodeName.toLowerCase() === 'object') {
                            callbackObjectRemoved(node);
                        }
                    }
                }
            }
        });
        const config = {
            childList: true,
            subtree: true,
        };
        observer.observe(document.documentElement || document.body, config);
    }

    // Override createElement install a proxy to monitor and intercept objects. Needed for the
    // case where a page script creates and uses an object before adding it to the document.
    function addCreateElementTypeIntercept(callbackTypeSet) {
        const ownProperty = Object.getOwnPropertyDescriptor(HTMLObjectElement.prototype, 'type');
        Object.defineProperty(HTMLObjectElement.prototype, 'type', {
            set(val) {
                this.setAttribute('type', val);
            },
            get() {
                return ownProperty.get.call(this);
            },
        });

        HTMLObjectElement.prototype.setAttribute = function(name, value) {
            if (name === 'type') {
                callbackTypeSet(this, value);
            }
            Element.prototype.setAttribute.apply(this, arguments);
        };

        document.createElement = function(tagname, options) {
            let element = __createElement.apply(document, arguments);
            return element;
        };
    }

    // Intercept direct HTML manipulation. Needed for the (possibly esoteric) case where a page
    // script directly modifies the HTML to create an object and immediately uses it (i.e. before
    // the event loop processes mutation events).
    function addHTMLManipulationIntercept(interceptCallback) {
        Object.defineProperty(HTMLElement.prototype, 'innerHTML', {
            set: function(val) {
                const ownProperty = Object.getOwnPropertyDescriptor(Element.prototype, 'innerHTML');
                ownProperty.set.call(this, val);
                interceptCallback(this);
            },
            get: function() {
                const ownProperty = Object.getOwnPropertyDescriptor(Element.prototype, 'innerHTML');
                return ownProperty.get.call(this);
            },
        });
        Object.defineProperty(HTMLElement.prototype, 'outerHTML', {
            set: function(val) {
                const parent = this.parentElement;
                const ownProperty = Object.getOwnPropertyDescriptor(Element.prototype, 'outerHTML');
                ownProperty.set.call(this, val);
                interceptCallback(parent);
            },
            get: function() {
                const ownProperty = Object.getOwnPropertyDescriptor(Element.prototype, 'outerHTML');
                return ownProperty.get.call(this);
            },
        });
        HTMLElement.prototype.insertAdjacentHTML = function(position, text) {
            const result = Element.prototype.insertAdjacentHTML.call(this, position, text);
            interceptCallback(this.parentElement);
            return result;
        };
    }

    function addOipfObjectFactory() {
        if (window.oipfObjectFactory !== undefined) {
            console.error('Cannot redefine oipfObjectFactory!');
            return;
        }

        window.oipfObjectFactory = {
            isObjectSupported: function(objectType) {
                return resolveMimeType(objectType.toLowerCase()) !== undefined;
            },
        };

        for (let objectFactoryMethod in objectFactoryMethodTable) {
            console.log(objectFactoryMethod);
            const objectName = objectFactoryMethodTable[objectFactoryMethod];
            window.oipfObjectFactory[objectFactoryMethod] = function() {
                const object = __createElement.call(document, 'object');
                object.setAttribute('type', objectName);
                objectUpgradeHandlers[objectName](object);
                return object;
            };
        }
    }

    function createRdkVideoElement() {
        console.log('[RDK] Creating video element for holepunch');
        const video = __createElement.call(document, 'video');
        video._rdkHolepunch = true;
        return video;
    }

    function addMetaViewportElement() {
        console.log("addMetaViewportElement");
        if (!document.querySelector('meta[name=viewport]')) {
            // console.log("addMetaViewportElement");
            let meta = document.createElement('meta');
            meta.name = 'viewport';
            meta.content = 'width=device-width, initial-scale=1.0';
            document.getElementsByTagName('head')[0] ?.appendChild(meta);
            // document.head.appendChild(meta)
            // console.log(document.getElementsByTagName('head').);
            // console.log(document.getElementsByTagName('head')[1]);
            // console.log(document.head.title);
            // console.log(document.head.meta.content);
        }
        // var headContent = document.head.children; // Get all child nodes of the head element
        // for (var i = 0; i < headContent.length; i++) {
        //     console.log(headContent[i].outerHTML); // Output the outer HTML of each child node
        // }
    }
    
    return {
        initialise: initialise,
        registerObject: registerObject,
        createRdkVideoElement: createRdkVideoElement,
    };
})();