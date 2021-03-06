/*
* Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2017 Yermalayeu Ihar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#ifndef __SimdMotion_hpp__
#define __SimdMotion_hpp__

#include "Simd/SimdPoint.hpp"
#include "Simd/SimdRectangle.hpp"
#include "Simd/SimdFrame.hpp"
#include "Simd/SimdDrawing.hpp"
#include "Simd/SimdFont.hpp"

#include <vector>
#include <stack>
#include <sstream>

#ifndef SIMD_CHECK_PERFORMANCE
#define SIMD_CHECK_PERFORMANCE()
#endif

namespace Simd
{
    namespace Motion
    {
        typedef double Time;
        typedef int Id;
        typedef std::string String;
        typedef Simd::Point<ptrdiff_t> Size;
        typedef Simd::Point<ptrdiff_t> Point;
        typedef std::vector<Point> Points;
        typedef Simd::Point<double> FSize;
        typedef Simd::Point<double> FPoint;
        typedef std::vector<FPoint> FPoints;
        typedef Simd::Rectangle<ptrdiff_t> Rect;
        typedef std::vector<Rect> Rects;
        typedef Simd::Rectangle<double> FRect;
        typedef Simd::View<Simd::Allocator> View;
        typedef Simd::Frame<Simd::Allocator> Frame;
        typedef Simd::Pyramid<Simd::Allocator> Pyramid;

        SIMD_INLINE double ScreenToOnvifX(ptrdiff_t x, ptrdiff_t width)
        {
            return double(2 * x - width) / width;
        }

        SIMD_INLINE double ScreenToOnvifY(ptrdiff_t y, ptrdiff_t height)
        {
            return double(height - 2 * y) / height;
        }

        SIMD_INLINE FPoint ScreenToOnvif(const Point & screen, const Point & size)
        {
            return FPoint(ScreenToOnvifX(screen.x, size.x), ScreenToOnvifY(screen.y, size.y));
        }

        SIMD_INLINE FSize ScreenToOnvifSize(const Size & screen, const Point & size)
        {
            return FSize(double(screen.x * 2 / size.x), double(screen.y * 2 / size.y));
        }

        SIMD_INLINE ptrdiff_t OnvifToScreenX(double x, ptrdiff_t width)
        {
            return std::max(ptrdiff_t(0), std::min(width - 1, (ptrdiff_t)Simd::Round((1.0 + x)*width / 2.0)));
        }

        SIMD_INLINE ptrdiff_t OnvifToScreenY(double y, ptrdiff_t height)
        {
            return std::max(ptrdiff_t(0), std::min(height - 1, (ptrdiff_t)Simd::Round((1.0 - y)*height / 2.0)));
        }

        SIMD_INLINE Point OnvifToScreen(const FPoint & onvif, const Point & size)
        {
            return Point(OnvifToScreenX(onvif.x, size.x), OnvifToScreenY(onvif.y, size.y));
        }

        SIMD_INLINE Size OnvifToScreenSize(const FSize & onvif, const Point & size)
        {
            return Size(Round(onvif.x*size.x / 2.0), Round(onvif.y*size.y / 2.0));
        }

        SIMD_INLINE String ToString(Id id)
        {
            std::stringstream ss;
            ss << id;
            return ss.str();
        }

        struct Position
        {
            Point point;
            Time time;
        };
        typedef std::vector<Position> Positions;

        struct Object
        {
            Id id;
            Rect rect;
            Positions trajectory;
        };
        typedef std::vector<Object> Objects;

        struct Event
        {
            enum Type
            {
                ObjectIn,
                ObjectOut,
                SabotageOn,
                SabotageOff,
            } type;

            String text;
            Id objectId;

            Event(Type type_, const String & text_ = String(), Id objectId_ = -1)
                : type(type_)
                , text(text_)
                , objectId(objectId_)
            {
            }
        };
        typedef std::vector<Event> Events;

        struct Metadata
        {
            Objects objects;
            Events events;
        };

        struct Model
        {
            FSize size; // Minimal detected object size (Onvif coordinates).
            FPoints roi; // ROI (Onvif coordinates). 

            Model(const Model & m)
                : size(m.size)
                , roi(m.roi)
            {
            }

            Model(const FSize & s = FSize(0.1, 0.1), const FPoints & r = FPoints())
                : size(s)
                , roi(r)
            {
                if (roi.size() < 3)
                {
                    roi.clear();
                    roi.push_back(FPoint(-1.0, 1.0));
                    roi.push_back(FPoint(1.0, 1.0));
                    roi.push_back(FPoint(1.0, -1.0));
                    roi.push_back(FPoint(-1.0, -1.0));
                }
            }
        };

        struct Options
        {
            int CalibrationScaleLevelMax;
            int CalibrationLevelCountMin;
            int CalibrationTopLevelSizeMin;
            int CalibrationObjectAreaMin;

            int TextureGradientSaturation;
            int TextureGradientBoost;

            int DifferenceGrayFeatureWeight;
            int DifferenceDxFeatureWeight;
            int DifferenceDyFeatureWeight;
            bool DifferencePropagateForward;
            bool DifferenceRoiMaskEnable;

            double BackgroundGrowTime;
            double BackgroundIncrementTime;
            int BackgroundSabbotageCountMax;

            double SegmentationCreateThreshold;
            double SegmentationExpandCoefficient;

            double StabilityRegionAreaMax;

            int TrackingTrajectoryMax;
            double TrackingRemoveTime;
            double TrackingAdditionalLinking;
            int TrackingAveragingHalfRange;

            double ClassificationShiftMin;
            double ClassificationTimeMin;

            int DebugDrawLevel;
            int DebugDrawBottomRight; // 0 - empty; 1 = difference; 2 - texture.gray.value; 3 - texture.dx.value; 4 - texture.dy.value;
            bool DebugAnnotateModel;
            bool DebugAnnotateMovingRegions;
            bool DebugAnnotateTrackingObjects;

            Options()
            {
                CalibrationScaleLevelMax = 3;
                CalibrationLevelCountMin = 3;
                CalibrationTopLevelSizeMin = 32;
                CalibrationObjectAreaMin = 16;

                TextureGradientSaturation = 16;
                TextureGradientBoost = 4;

                DifferenceGrayFeatureWeight = 18;
                DifferenceDxFeatureWeight = 18;
                DifferenceDyFeatureWeight = 18;
                DifferencePropagateForward = true;
                DifferenceRoiMaskEnable = true;

                BackgroundGrowTime = 1.0;
                BackgroundIncrementTime = 1.0;
                BackgroundSabbotageCountMax = 3;

                SegmentationCreateThreshold = 0.5;
                SegmentationExpandCoefficient = 0.75;

                StabilityRegionAreaMax = 0.5;

                TrackingTrajectoryMax = 1024;
                TrackingRemoveTime = 1.0;
                TrackingAdditionalLinking = 0.0;
                TrackingAveragingHalfRange = 12;

                ClassificationShiftMin = 0.075;
                ClassificationTimeMin = 1.0;

                DebugDrawLevel = 1;
                DebugDrawBottomRight = 0;
                DebugAnnotateModel = false;
                DebugAnnotateMovingRegions = false;
                DebugAnnotateTrackingObjects = false;
            }
        };

        struct Detector
        {
            Detector()
            {
            }

            ~Detector()
            {
            }

            bool SetOptions(const Options & options)
            {
                _options = options;
                return true;
            }

            bool SetModel(const Model & model)
            {
                _model = model;
                return true;
            }

            bool NextFrame(const Frame & input, Metadata & metadata, Frame * output = NULL)
            {
                SIMD_CHECK_PERFORMANCE();

                if (output && output->Size() != input.Size())
                    return false;

                if (!Calibrate(input.Size()))
                    return false;

                _scene.metadata = &metadata;
                _scene.metadata->events.clear();

                SetFrame(input, output);

                EstimateTextures();

                EstimateDifference();

                PerformSegmentation();

                VerifyStability();

                TrackObjects();

                ClassifyObjects();

                UpdateBackground();

                SetMetadata();

                DebugAnnotation();

                return true;
            }

        private:
            Options _options;
            Simd::Motion::Model _model;

            typedef std::pair<size_t, size_t> Scanline;
            typedef std::vector<Scanline> Scanlines;

            struct SearchRegion
            {
                Rect rect; // rectangle on corresponding pyramid level (scale)
                int scale; // pyramid level
                Scanlines scanlines;

                SearchRegion()
                    : scale(0)
                {
                }

                SearchRegion(const Rect & rect_, const int & scale_)
                    : rect(rect_)
                    , scale(scale_)
                {
                }
            };
            typedef std::vector<SearchRegion> SearchRegions;

            struct Model
            {
                Size originalFrameSize;

                Size frameSize;
                size_t scale;
                size_t scaleLevel;

                size_t levelCount;
                int areaRegionMinEstimated;

                Points roi;
                Pyramid roiMask;
                SearchRegions searchRegions;
            };

            struct Object;

            struct MovingRegion
            {
                Rects rects;

                uint8_t index;
                Rect rect;
                int level;
                Time time;
                Point point;
                Detector::Object * object, * nearest;

                MovingRegion(const uint8_t & index_, const Rect & rect_, int level_, const Time & time_)
                    : index(index_)
                    , rect(rect_)
                    , level(level_)
                    , time(time_)
                    , object(NULL)
                    , nearest(NULL)
                {
                    rects.resize(level + 1);
                }
            };
            typedef std::shared_ptr<MovingRegion> MovingRegionPtr;
            typedef std::vector<MovingRegionPtr> MovingRegionPtrs;

            struct Texture
            {
                struct Bound
                {
                    Pyramid value;
                    Pyramid count;

                    void Create(const Size & size, size_t levelCount)
                    {
                        value.Recreate(size, levelCount);
                        count.Recreate(size, levelCount);
                    }
                };

                struct Feature
                {
                    Pyramid value;
                    Bound lo;
                    Bound hi;
                    uint16_t weight;

                    void Create(const Size & size, size_t levelCount, int weight_)
                    {
                        value.Recreate(size, levelCount);
                        lo.Create(size, levelCount);
                        hi.Create(size, levelCount);
                        weight = uint16_t(weight_ * 256);
                    }
                };

                enum FeatureType
                {
                    FeatureGray,
                    FeatureDx,
                    FeatureDy,
                };

                Feature gray;
                Feature dx;
                Feature dy;

                typedef std::vector<Feature *> Features;
                Features features;

                void Create(const Size & size, size_t levelCount, const Options & options)
                {
                    gray.Create(size, levelCount, options.DifferenceGrayFeatureWeight);
                    dx.Create(size, levelCount, options.DifferenceDxFeatureWeight);
                    dy.Create(size, levelCount, options.DifferenceDyFeatureWeight);

                    features.clear();
                    features.push_back(&gray);
                    features.push_back(&dx);
                    features.push_back(&dy);
                }
            };

            struct Background
            {
                enum State
                {
                    Init,
                    Grow,
                    Update
                };

                State state;
                int count;
                int sabotageCounter;
                Time growEndTime;
                Time lastFrameTime;
                Time incrementCounterTime;

                Background()
                    : state(Init)
                {
                }
            };

            struct Stability
            {
                enum State
                {
                    Stable,
                    Sabotage
                } state;

                Stability()
                    : state(Stable)
                {
                }
            };

            struct Segmentation
            {
                enum MaskIndices
                {
                    MaskNotVisited = 0,
                    MaskSeed = 1,
                    MaskInvalid = 2,
                    MaskIndexSize,
                };

                Pyramid mask;

                int differenceCreationMin;
                int differenceExpansionMin;

                MovingRegionPtrs movingRegions;
            };

            struct Object
            {
                Id trackingId, classificationId;
                Point center; 
                Rect rect; 
                MovingRegionPtrs trajectory; 

                enum Type
                {
                    Static,
                    Moving,
                } type;

                Point pointStart;
                Time timeStart;

                Object(const Id trackingId_, const MovingRegionPtr & region)
                    : trackingId(trackingId_)
                    , classificationId(-1)
                    , center(region->rect.Center())
                    , rect(region->rect)
                    , type(Static)
                    , pointStart(region->rect.Center())
                    , timeStart(region->time)
                {
                    trajectory.push_back(region);
                }
            };
            typedef std::shared_ptr<Object> ObjectPtr;
            typedef std::vector<ObjectPtr> ObjectPtrs;

            struct Tracking
            {
                ObjectPtrs objects;
                ObjectPtrs justDeletedObjects;
                Id id; 

                Tracking() 
                    : id(0)
                {
                }
            };

            struct Classification
            {
                ptrdiff_t squareShiftMin;
                Id id;

                Classification()
                    : id(0)
                {
                }
            };

            struct Scene
            {
                Frame input, * output;
                Pyramid scaled;
                Metadata * metadata;

                Font font;
                Pyramid buffer;
                Detector::Model model;

                Texture texture;

                Background background;

                Stability stability;

                Pyramid difference;

                Segmentation segmentation;

                Tracking tracking;

                Classification classification;

                void Create(const Options & options)
                {
                    scaled.Recreate(model.originalFrameSize, model.scaleLevel + 1);
                    font.Resize(model.originalFrameSize.y / 32);
                    buffer.Recreate(model.frameSize, model.levelCount);

                    texture.Create(model.frameSize, model.levelCount, options);
                    difference.Recreate(model.frameSize, model.levelCount);

                    segmentation.mask.Recreate(model.frameSize, model.levelCount);
                    segmentation.differenceCreationMin = int(255 * options.SegmentationCreateThreshold);
                    segmentation.differenceExpansionMin = int(255 * options.SegmentationExpandCoefficient*options.SegmentationCreateThreshold);

                    classification.squareShiftMin = ptrdiff_t(Simd::SquaredDistance(model.frameSize, Point())*
                        options.ClassificationShiftMin*options.ClassificationShiftMin);
                }
            };

            void SetFrame(const Frame & input, Frame * output)
            {
                SIMD_CHECK_PERFORMANCE();

                _scene.input = input;
                _scene.output = output;
                Simd::Convert(input, Frame(_scene.scaled[0]).Ref());
                Simd::Build(_scene.scaled, SimdReduce2x2);
            }

            bool Calibrate(const Size & frameSize)
            {
                Model & model = _scene.model;

                if (model.originalFrameSize == frameSize)
                    return true;

                SIMD_CHECK_PERFORMANCE();

                model.originalFrameSize = frameSize;

                EstimateModelParameters(model);
                SetScreenRoi(model);
                GenerateSearchRegion(model);
                GenerateSearchRegionScanlines(model);

                _scene.Create(_options);

                return true;
            }

            void EstimateModelParameters(Model & model)
            {
                Size objectSize = OnvifToScreenSize(_model.size, model.originalFrameSize);
                Size size = model.originalFrameSize;
                model.areaRegionMinEstimated = int(objectSize.x*objectSize.y);
                int levelCount = 1;
                while (size.x >= _options.CalibrationTopLevelSizeMin && size.y >= _options.CalibrationTopLevelSizeMin && model.areaRegionMinEstimated > _options.CalibrationObjectAreaMin)
                {
                    size = Simd::Scale(size);
                    ++levelCount;
                    model.areaRegionMinEstimated /= 4;
                }
                model.areaRegionMinEstimated = std::max(model.areaRegionMinEstimated, _options.CalibrationObjectAreaMin / 4 + 1);
                model.scaleLevel = std::min(std::max(levelCount - _options.CalibrationLevelCountMin, 0), _options.CalibrationScaleLevelMax);
                model.levelCount = levelCount - model.scaleLevel;
                model.scale = size_t(1) << model.scaleLevel;
                model.frameSize = model.originalFrameSize;
                for (size_t level = 0; level < model.scaleLevel; ++level)
                    model.frameSize = Simd::Scale(model.frameSize);
            }

            void SetScreenRoi(Model & model)
            {
                if (_model.roi.size() > 2)
                {
                    model.roi.resize(_model.roi.size());
                    for (size_t i = 0; i < _model.roi.size(); ++i)
                        model.roi[i] = OnvifToScreen(_model.roi[i], model.frameSize);
                }
                else
                {
                    model.roi.clear();
                    model.roi.push_back(Point(0, 0));
                    model.roi.push_back(Point(model.frameSize.x, 0));
                    model.roi.push_back(Point(model.frameSize.x, model.frameSize.y));
                    model.roi.push_back(Point(0, model.frameSize.y));
                }
            }

            void GenerateSearchRegion(Model & model)
            {
                model.searchRegions.clear();

                Size size(model.frameSize);
                for (size_t level = 1; level < model.levelCount; level++)
                    size = Simd::Scale(size);

                int level = (int)model.levelCount - 1;
                const Rect rect(1, 1, size.x - 1, size.y - 1);
                model.searchRegions.push_back(SearchRegion(rect, level));
            }

            void GenerateSearchRegionScanlines(Model & model)
            {
                static const uint8_t ROI_EMPTY = 0;
                static const uint8_t ROI_NON_EMPTY = 255;

                model.roiMask.Recreate(model.frameSize, model.levelCount);
                Simd::Fill(model.roiMask, ROI_EMPTY);
                DrawFilledPolygon(model.roiMask[0], model.roi, ROI_NON_EMPTY);
                Simd::Build(model.roiMask, SimdReduce4x4);

                for (size_t i = 0; i < model.searchRegions.size(); ++i)
                {
                    SearchRegion & region = model.searchRegions[i];
                    assert(region.scale < (int)model.roiMask.Size());

                    const View & view = model.roiMask[region.scale];
                    const Rect & rect = region.rect;
                    for (ptrdiff_t row = rect.Top(); row < rect.Bottom(); ++row)
                    {
                        ptrdiff_t offset = row * view.stride + rect.Left();
                        ptrdiff_t end = offset + rect.Width();
                        for (; offset < end;)
                        {
                            if (view.data[offset])
                            {
                                Scanline scanline;
                                scanline.first = offset;
                                while (++offset < end && view.data[offset]);
                                scanline.second = offset;
                                region.scanlines.push_back(scanline);
                            }
                            else
                                ++offset;
                        }
                    }
                }
            }

            void EstimateTextures()
            {
                SIMD_CHECK_PERFORMANCE();

                Texture & texture = _scene.texture;
                Simd::Copy(_scene.scaled.Top(), texture.gray.value[0]);
                Simd::Build(texture.gray.value, SimdReduce4x4);
                for (size_t i = 0; i < texture.gray.value.Size(); ++i)
                {
                    Simd::TextureBoostedSaturatedGradient(texture.gray.value[i],
                        _options.TextureGradientSaturation, _options.TextureGradientBoost,
                        texture.dx.value[i], texture.dy.value[i]);
                }
            }

            void EstimateDifference()
            {
                SIMD_CHECK_PERFORMANCE();

                const Texture & texture = _scene.texture;
                Pyramid & difference = _scene.difference;
                Pyramid & buffer = _scene.buffer;
                for (size_t i = 0; i < difference.Size(); ++i)
                {
                    Simd::Fill(difference[i], 0);
                    for (size_t j = 0; j < texture.features.size(); ++j)
                    {
                        const Texture::Feature & feature = *texture.features[j];
                        Simd::AddFeatureDifference(feature.value[i], feature.lo.value[i], feature.hi.value[i], feature.weight, difference[i]);
                    }
                }
                if (_options.DifferencePropagateForward)
                {
                    for (size_t i = 1; i < difference.Size(); ++i)
                    {
                        Simd::ReduceGray4x4(difference[i - 1], buffer[i]);
                        Simd::OperationBinary8u(difference[i], buffer[i], difference[i], SimdOperationBinary8uMaximum);
                    }
                }
                if (_options.DifferenceRoiMaskEnable)
                {
                    for (size_t i = 0; i < difference.Size(); ++i)
                        Simd::OperationBinary8u(difference[i], _scene.model.roiMask[i], difference[i], SimdOperationBinary8uAnd);
                }
            }

            void PerformSegmentation()
            {
                SIMD_CHECK_PERFORMANCE();

                Point neighbours[4];
                neighbours[0] = Point(-1, 0);
                neighbours[1] = Point(0, -1);
                neighbours[2] = Point(1, 0);
                neighbours[3] = Point(0, 1);

                Segmentation & segmentation = _scene.segmentation;
                const Model & model = _scene.model;
                const Time & time = _scene.input.timestamp;

                segmentation.movingRegions.clear();

                Simd::Fill(segmentation.mask, Segmentation::MaskNotVisited);
                for (size_t i = 0; i < model.searchRegions.size(); ++i)
                {
                    View & mask = segmentation.mask.At(model.searchRegions[i].scale);
                    Simd::FillFrame(mask, Rect(1, 1, mask.width - 1, mask.height - 1), Segmentation::MaskInvalid);
                }

                for (size_t i = 0; i < model.searchRegions.size(); ++i)
                {
                    const SearchRegion & searchRegion = model.searchRegions[i];
                    int level = searchRegion.scale;
                    const View & difference = _scene.difference.At(level);
                    View & mask = segmentation.mask.At(level);
                    Rect roi = searchRegion.rect;

                    for (size_t i = 0; i < searchRegion.scanlines.size(); ++i)
                    {
                        const Scanline & scanline = searchRegion.scanlines[i];
                        for (size_t offset = scanline.first; offset < scanline.second; ++offset)
                        {
                            if (difference.data[offset] > segmentation.differenceCreationMin && mask.data[offset] == Segmentation::MaskNotVisited)
                                mask.data[offset] = Segmentation::MaskSeed;
                        }
                    }

                    ShrinkRoi(mask, roi, Segmentation::MaskSeed);
                    roi &= searchRegion.rect;

                    for (ptrdiff_t y = roi.top; y < roi.bottom; ++y)
                    {
                        for (ptrdiff_t x = roi.left; x < roi.right; ++x)
                        {
                            if (mask.At<uint8_t>(x, y) == Segmentation::MaskSeed)
                            {
                                std::stack<Point> stack;
                                stack.push(Point(x, y));
                                if (segmentation.movingRegions.size() + Segmentation::MaskIndexSize > UINT8_MAX)
                                    return;
                                MovingRegionPtr region(new MovingRegion(uint8_t(segmentation.movingRegions.size() + Segmentation::MaskIndexSize), Rect(), level, time));
                                while (!stack.empty())
                                {
                                    Point current = stack.top();
                                    stack.pop();
                                    mask.At<uint8_t>(current) = region->index;
                                    region->rect |= current;
                                    for (size_t n = 0; n < 4; ++n)
                                    {
                                        Point neighbour = current + neighbours[n];
                                        if (difference.At<uint8_t>(neighbour) > segmentation.differenceExpansionMin &&	mask.At<uint8_t>(neighbour) <= Segmentation::MaskSeed)
                                            stack.push(neighbour);
                                    }
                                }

                                if (region->rect.Area() <= model.areaRegionMinEstimated)
                                    Simd::SegmentationChangeIndex(segmentation.mask[region->level].Region(region->rect).Ref(), region->index, Segmentation::MaskInvalid);
                                else
                                {
                                    ComputeIndex(segmentation, *region);
                                    if (!region->rect.Empty())
                                    {
                                        region->level = searchRegion.scale;
                                        region->point = region->rect.Center();
                                        segmentation.movingRegions.push_back(region);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            SIMD_INLINE void ShrinkRoi(const View & mask, Rect & roi, uint8_t index)
            {
                Simd::SegmentationShrinkRegion(mask, index, roi);
                if (!roi.Empty())
                    roi.AddBorder(1);
            }

            SIMD_INLINE void ExpandRoi(const Rect & roiParent, const Rect & rectChild, Rect & roiChild)
            {
                roiChild.SetTopLeft(roiParent.TopLeft() * 2 - Point(1, 1));
                roiChild.SetBottomRight(roiParent.BottomRight() * 2 + Point(1, 1));
                roiChild.AddBorder(1);
                roiChild &= rectChild;
            }

            void ComputeIndex(const View & parentMask, View & childMask, const View & difference, MovingRegion & region, int differenceExpansionMin)
            {
                Rect rect = region.rect;
                rect.right++;
                rect.bottom++;
                Simd::SegmentationPropagate2x2(parentMask.Region(rect), childMask.Region(2 * rect).Ref(), difference.Region(2 * rect),
                    region.index, Segmentation::MaskInvalid, Segmentation::MaskNotVisited, differenceExpansionMin);

                Rect rectChild(childMask.Size());
                rectChild.AddBorder(-1);

                ExpandRoi(region.rect, rectChild, region.rect);
                ShrinkRoi(childMask, region.rect, region.index);
                region.rect &= rectChild;
            }

            void ComputeIndex(Segmentation & segmentation, MovingRegion & region)
            {
                region.rects[region.level] = region.rect;

                int level = region.level;
                std::stack<Rect> rects;
                for (; region.level > 0; --region.level)
                {
                    const int levelChild = region.level - 1;

                    rects.push(region.rect);
                    ComputeIndex(segmentation.mask[region.level], segmentation.mask[levelChild], _scene.difference[levelChild], region, segmentation.differenceExpansionMin);

                    region.rects[region.level - 1] = region.rect;

                    if (region.rect.Empty())
                    {
                        for (; region.level <= level; region.level++)
                        {
                            region.rect = rects.top();
                            rects.pop();
                            Simd::SegmentationChangeIndex(segmentation.mask[region.level].Region(region.rect).Ref(), region.index, Segmentation::MaskInvalid);
                        }
                        region.rect = Rect();
                        return;
                    }
                }
            }

            void VerifyStability()
            {
                SIMD_CHECK_PERFORMANCE();

                if (_scene.background.state == Background::Init)
                    return;
                View mask = _scene.segmentation.mask[0];
                uint32_t count;
                Simd::ConditionalCount8u(mask, Segmentation::MaskIndexSize, SimdCompareGreaterOrEqual, count);
                bool sabotage = count >= mask.Area()*_options.StabilityRegionAreaMax;
                if (sabotage)
                {
                    if (_scene.stability.state != Stability::Sabotage)
                        _scene.metadata->events.push_back(Event(Event::SabotageOn, "SabotageOn"));
                    _scene.stability.state = Stability::Sabotage;
                }
                else
                {
                    if (_scene.stability.state == Stability::Sabotage)
                        _scene.metadata->events.push_back(Event(Event::SabotageOff, "SabotageOff"));
                    _scene.stability.state = Stability::Stable;
                }
            }

            void TrackObjects()
            {
                SIMD_CHECK_PERFORMANCE();

                if (_scene.background.state != Background::Update)
                {
                    RemoveAllObjects();
                    return;
                }

                RefreshObjectsTrajectory();

                DeleteOldObjects();

                SetNearestObjects();

                LinkObjects();

                AddNewObjects();
            }

            void RemoveAllObjects()
            {
                _scene.tracking.justDeletedObjects.clear();
                if (_scene.tracking.objects.size())
                {
                    for (size_t i = 0; i < _scene.tracking.objects.size(); ++i)
                    {
                        ObjectPtr & object = _scene.tracking.objects[i];
                        if (object->type == Object::Moving)
                            _scene.metadata->events.push_back(Event(Event::ObjectOut, "ObjectOut", object->classificationId));
                        _scene.tracking.justDeletedObjects.push_back(object);
                    }
                    _scene.tracking.objects.clear();
                }
            }

            void RefreshObjectsTrajectory()
            {
                ObjectPtrs & objects = _scene.tracking.objects;
                for (size_t j = 0; j < objects.size(); ++j)
                {
                    ObjectPtr & object = objects[j];
                    if (object->trajectory.size() > _options.TrackingTrajectoryMax)
                        object->trajectory.erase(object->trajectory.begin());
                }
            }

            void DeleteOldObjects()
            {
                Time current = _scene.input.timestamp;
                Tracking & tracking = _scene.tracking;
                tracking.justDeletedObjects.clear();
                ObjectPtrs buffer;
                for (size_t i = 0; i < tracking.objects.size(); ++i)
                {
                    const ObjectPtr & object = tracking.objects[i];
                    if (current - object->trajectory.back()->time < _options.TrackingRemoveTime)
                        buffer.push_back(object);
                    else
                    {
                        tracking.justDeletedObjects.push_back(object);
                        if (object->type == Object::Moving)
                            _scene.metadata->events.push_back(Event(Event::ObjectOut, "ObjectOut", object->classificationId));
                    }
                }
                tracking.objects.swap(buffer);
            }

            void SetNearestObjects()
            {
                for (size_t i = 0; i < _scene.segmentation.movingRegions.size(); ++i)
                {
                    MovingRegion & region = *_scene.segmentation.movingRegions[i];
                    region.nearest = NULL;
                    ptrdiff_t minDifferenceSquared = std::numeric_limits<ptrdiff_t>::max();
                    for (size_t j = 0; j < _scene.tracking.objects.size(); ++j)
                    {
                        Detector::Object * object = _scene.tracking.objects[j].get();
                        const ptrdiff_t differenceSquared = Simd::SquaredDistance(object->center, region.rect.Center());
                        if (differenceSquared < minDifferenceSquared)
                        {
                            minDifferenceSquared = differenceSquared;
                            region.nearest = object;
                        }
                    }
                }
            }

            void LinkObjects()
            {
                for (size_t i = 0; i < _scene.tracking.objects.size(); ++i)
                {
                    ObjectPtr & object = _scene.tracking.objects[i];
                    MovingRegionPtr nearest;
                    ptrdiff_t minDifferenceSquared = std::numeric_limits<ptrdiff_t>::max();
                    for (size_t j = 0; j < _scene.segmentation.movingRegions.size(); ++j)
                    {
                        MovingRegionPtr & region = _scene.segmentation.movingRegions[j];
                        if (region->object != NULL)
                            continue;
                        if (object.get() != region->nearest)
                            continue;
                        Rect regionRect = Enlarged(region->rect);
                        Rect objectRect = Enlarged(object->rect);
                        ptrdiff_t differenceSquared = Simd::SquaredDistance(object->center, region->rect.Center());
                        if (regionRect.Contains(object->center) || objectRect.Contains(region->rect.Center()))
                        {
                            if (differenceSquared < minDifferenceSquared)
                            {
                                minDifferenceSquared = differenceSquared;
                                nearest = region;
                            }
                        }
                    }
                    if (nearest)
                    {
                        nearest->object = object.get();
                        object->trajectory.push_back(nearest);
                        Rect sum;
                        size_t end = object->trajectory.size(), start = std::max<ptrdiff_t>(0, end - _options.TrackingAveragingHalfRange);
                        for (size_t j = start; j < end; ++j)
                            sum += object->trajectory[j]->rect;
                        object->rect = sum / (end - start);
                        object->rect.Shift(nearest->rect.Center() - object->rect.Center());
                        object->rect &= Rect(_scene.model.frameSize);
                        object->center = nearest->rect.Center();
                    }
                }
            }

            SIMD_INLINE Rect Enlarged(Rect rect)
            {
                ptrdiff_t size = (rect.Width() + rect.Height()) / 2;
                ptrdiff_t border = ptrdiff_t(::ceil(size*_options.TrackingAdditionalLinking));
                rect.AddBorder(border);
                return rect;
            }

            void AddNewObjects()
            {
                for (size_t j = 0; j < _scene.segmentation.movingRegions.size(); ++j)
                {
                    const MovingRegionPtr & region = _scene.segmentation.movingRegions[j];
                    if (region->object != NULL)
                        continue;
                    bool contained = false;
                    for (size_t i = 0; i < _scene.tracking.objects.size(); ++i)
                    {
                        const ObjectPtr & object = _scene.tracking.objects[i];
                        if (object->rect.Contains(region->rect.Center()))
                        {
                            contained = true;
                            break;
                        }
                    }
                    if (!contained)
                    {
                        ObjectPtr object(new Object(_scene.tracking.id++, region));
                        region->object = object.get();
                        _scene.tracking.objects.push_back(object);
                    }
                }
            }

            void ClassifyObjects()
            {
                for (size_t i = 0; i < _scene.tracking.objects.size(); ++i)
                {
                    Object & object = *_scene.tracking.objects[i];
                    if (object.type == Object::Static)
                    {
                        Time time = _scene.input.timestamp - object.timeStart;
                        ptrdiff_t squareShift = Simd::SquaredDistance(object.trajectory.back()->point, object.pointStart);
                        if (time >= _options.ClassificationTimeMin && squareShift >= _scene.classification.squareShiftMin)
                        {
                            object.type = Object::Moving;
                            object.classificationId = _scene.classification.id++;
                            _scene.metadata->events.push_back(Event(Event::ObjectIn, "ObjectIn", object.classificationId));
                        }
                    }
                }
            }

            struct InitUpdater
            {
                void operator()(View & value, View & loValue, View & loCount, View & hiValue, View & hiCount) const
                {
                    Simd::Copy(value, loValue);
                    Simd::Copy(value, hiValue);
                    Simd::Fill(loCount, 0);
                    Simd::Fill(hiCount, 0);
                }
            };

            struct GrowRangeUpdater
            {
                void operator()(View & value, View & loValue, View & loCount, View & hiValue, View & hiCount) const
                {
                    Simd::BackgroundGrowRangeFast(value, loValue, hiValue);
                }
            };

            struct IncrementCountUpdater
            {
                void operator()(View & value, View & loValue, View & loCount, View & hiValue, View & hiCount) const
                {
                    Simd::BackgroundIncrementCount(value, loValue, hiValue, loCount, hiCount);
                }
            };

            struct AdjustRangeUpdater
            {
                void operator()(View & value, View & loValue, View & loCount, View & hiValue, View & hiCount) const
                {
                    Simd::BackgroundAdjustRange(loCount, loValue, hiCount, hiValue, 1);
                }
            };

            template <typename Updater> void Apply(Texture::Features & features, const Updater & updater)
            {
                for (size_t i = 0; i < features.size(); ++i)
                {
                    Texture::Feature & feature = *features[i];
                    for (size_t j = 0; j < feature.value.Size(); ++j)
                    {
                        updater(feature.value[j], feature.lo.value[j], feature.lo.count[j], feature.hi.value[j], feature.hi.count[j]);
                    }
                }
            }

            void UpdateBackground()
            {
                SIMD_CHECK_PERFORMANCE();

                Background & background = _scene.background;
                const Stability::State & stability = _scene.stability.state;
                const Time & time = _scene.input.timestamp;
                switch (background.state)
                {
                case Background::Update:
                    switch (stability)
                    {
                    case Stability::Stable:
                        Apply(_scene.texture.features, IncrementCountUpdater());
                        ++background.count;
                        background.incrementCounterTime += time - background.lastFrameTime;
                        if (background.count >= CHAR_MAX || (background.incrementCounterTime > _options.BackgroundIncrementTime && background.count >= 8))
                        {
                            Apply(_scene.texture.features, AdjustRangeUpdater());
                            background.incrementCounterTime = 0;
                            background.count = 0;
                        }
                        break;
                    case Stability::Sabotage:
                        background.sabotageCounter++;
                        if (background.sabotageCounter > _options.BackgroundSabbotageCountMax)
                            InitBackground();
                        break;
                    default:
                        assert(0);
                    }
                    if (stability != Stability::Sabotage)
                        background.sabotageCounter = 0;
                    break;
                case Background::Grow:
                    if (stability == Stability::Sabotage)
                        InitBackground();
                    else
                    {
                        Apply(_scene.texture.features, GrowRangeUpdater());
                        if (stability != Stability::Stable)
                            background.growEndTime = time + _options.BackgroundGrowTime;
                        if (background.growEndTime < time)
                        {
                            background.state = Background::Update;
                            background.count = 0;
                        }
                    }
                    break;
                case Background::Init:
                    InitBackground();
                    break;
                default:
                    assert(0);
                }
                background.lastFrameTime = time;
            }

            void InitBackground()
            {
                Background & background = _scene.background;
                Apply(_scene.texture.features, InitUpdater());
                background.growEndTime = _scene.input.timestamp + _options.BackgroundGrowTime;
                background.state = Background::Grow;
                background.count = 0;
                background.incrementCounterTime = 0;
            }

            void SetMetadata()
            {
                _scene.metadata->objects.clear();
                AddToMetadata(_scene.tracking.objects);
                AddToMetadata(_scene.tracking.justDeletedObjects);
            }

            void AddToMetadata(const ObjectPtrs & objects)
            {
                size_t scale = _scene.model.scale;
                for (size_t i = 0; i < objects.size(); ++i)
                {
                    Object & srcObject = *objects[i];
                    if (srcObject.type == Object::Moving)
                    {
                        Motion::Object dstObject;
                        dstObject.id = srcObject.classificationId;
                        dstObject.rect = srcObject.rect*scale;
                        for (size_t j = 0; j < srcObject.trajectory.size(); ++j)
                        {
                            ptrdiff_t begin = std::max<ptrdiff_t>(0, j - _options.TrackingAveragingHalfRange);
                            ptrdiff_t end = std::min<ptrdiff_t>(srcObject.trajectory.size(), j + _options.TrackingAveragingHalfRange);
                            Point sum;
                            for (ptrdiff_t l = begin; l < end; ++l)
                                sum += srcObject.trajectory[l]->point*scale;
                            Motion::Position position;
                            position.time = srcObject.trajectory[j]->time;
                            position.point = sum / (end - begin);
                            dstObject.trajectory.push_back(position);
                        }
                        _scene.metadata->objects.push_back(dstObject);
                    }
                }
            }

            void DebugAnnotation()
            {
                SIMD_CHECK_PERFORMANCE();

                Frame * output = _scene.output;
                size_t scale = _scene.model.scale;

                if (output && output->format == Frame::Bgr24)
                {
                    View & canvas = output->planes[0];

                    if (_options.DebugDrawBottomRight)
                    {
                        View src;
                        switch (_options.DebugDrawBottomRight)
                        {
                        case 1: src = _scene.difference[_options.DebugDrawLevel]; break;
                        case 2: src = _scene.texture.gray.value[_options.DebugDrawLevel]; break;
                        case 3: src = _scene.texture.dx.value[_options.DebugDrawLevel]; break;
                        case 4: src = _scene.texture.dy.value[_options.DebugDrawLevel]; break;
                        }
                        Simd::GrayToBgr(src, canvas.Region(src.Size(), View::BottomRight).Ref());
                    }

                    if (_options.DebugAnnotateModel)
                    {
                        Simd::Pixel::Bgr24 color(0, 255, 255);
                        for (size_t i = 0; i < _scene.model.roi.size(); ++i)
                        {
                            Point p0 = i ? _scene.model.roi[i - 1] : _scene.model.roi.back(), p1 = _scene.model.roi[i];
                            Simd::DrawLine(canvas, p0*scale, p1*scale, color);
                        }
                        Rect objectMin(OnvifToScreenSize(_model.size, _scene.model.originalFrameSize));
                        objectMin.Shift(Point(_scene.model.originalFrameSize.x - objectMin.right - 2*scale, scale));
                        Simd::DrawRectangle(canvas, objectMin, color);
                    }

                    if (_options.DebugAnnotateMovingRegions)
                    {
                        Simd::Pixel::Bgr24 color(0, 255, 0);
                        for (size_t i = 0; i < _scene.segmentation.movingRegions.size(); ++i)
                        {
                            const MovingRegion & region = *_scene.segmentation.movingRegions[i];
                            Simd::DrawRectangle(canvas, region.rect*scale, color, 1);
                        }
                    }

                    if (_options.DebugAnnotateTrackingObjects)
                    {
                        Simd::Pixel::Bgr24 color(0, 255, 255);
                        for (size_t i = 0; i < _scene.tracking.objects.size(); ++i)
                        {
                            const Object & object = *_scene.tracking.objects[i];
                            Simd::DrawRectangle(canvas, object.rect*scale, color, 1);
                            _scene.font.Draw(canvas, ToString(object.trackingId), Point(object.rect.Center().x*scale, object.rect.top*scale - _scene.font.Height()), color);
                            const MovingRegionPtrs & regions = object.trajectory;
                            for (size_t j = 1; j < regions.size(); ++j)
                                Simd::DrawLine(canvas, regions[j]->point*scale, regions[j - 1]->point*scale, color, 1);
                        }
                    }
                }
            }

            Scene _scene;
        };
    }
}

#endif//__SimdMotion_hpp__
