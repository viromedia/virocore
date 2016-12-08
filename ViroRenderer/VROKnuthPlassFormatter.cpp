//
//  VROKnuthPlassFormatter.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 12/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROKnuthPlassFormatter.h"
#include <cmath>
#include <limits>
#include <algorithm>
#include "VROLog.h"

int kInfinity = 10000;

VROKnuthPlassFormatter::VROKnuthPlassFormatter(std::vector<std::shared_ptr<KPNode>> &nodes,
                                               std::vector<int> &lineLengths) :
    _nodes(nodes),
    _lineLengths(lineLengths) {
        
    _options.demerits.line = 10;
    _options.demerits.flagged = 100;
    _options.demerits.fitness = 3000;
    _options.tolerance = 2;
}

float VROKnuthPlassFormatter::computeCost(const KPSum &sumFromParentToNode,
                                          std::shared_ptr<KPBreakpoint> &parent,
                                          int currentLine) const {
    /*
     Get the maximum allowed width of the line. Use the last lineLength in the 
     array if we're exceeded the length of the array. Note this feature exists
     because we allow each line to have a different length.
     */
    float lineLength = currentLine < _lineLengths.size() ?
                            _lineLengths[currentLine - 1] :
                            _lineLengths[_lineLengths.size() - 1];
    
    /*
     If the width is less than the maxWidth, we have to stretch the whitespace
     to fill out the line. Compute the stretch ratio.
     */
    if (sumFromParentToNode.width < lineLength) {
        if (sumFromParentToNode.stretch > 0) {
            return (lineLength - sumFromParentToNode.width) / sumFromParentToNode.stretch;
        }
        else {
            return kInfinity;
        }
    }
    
    /*
     Otherwise we have to shrink the whitespace to fit in the line.
     */
    else if (sumFromParentToNode.width > lineLength) {
        if (sumFromParentToNode.shrink > 0) {
            return (lineLength - sumFromParentToNode.width) / sumFromParentToNode.shrink;
        }
        else {
            return kInfinity;
        }
    }
    
    /*
     Rare cases, we have a perfect match.
     */
    else {
        return 0;
    }
}

KPSum VROKnuthPlassFormatter::computeSum(const KPSum &sum, int breakpointIndex) const {
    KPSum result = { sum.width, sum.stretch, sum.shrink };
    
    for (int i = breakpointIndex; i < _nodes.size(); i++) {
        if (_nodes[i]->type == KPNodeType::Glue) {
            std::shared_ptr<KPGlue> glue = std::dynamic_pointer_cast<KPGlue>(_nodes[i]);
            result.width   += glue->width;
            result.stretch += glue->stretch;
            result.shrink  += glue->shrink;
        }
        else if (_nodes[i]->type == KPNodeType::Box) {
            break;
        }
        else if (_nodes[i]->type == KPNodeType::Penalty &&
                 std::dynamic_pointer_cast<KPPenalty>(_nodes[i])->penalty == -kInfinity &&
                 i > breakpointIndex) {
            
            break;
        }
    }
    return result;
}

std::vector<VROBreakpoint> VROKnuthPlassFormatter::run() {
    KPSum sum;
    std::list<std::shared_ptr<KPBreakpoint>> breakpoints;
    
    /*
     Add a breakpoint for the start of the paragraph.
     */
    std::shared_ptr<KPBreakpoint> empty;
    std::shared_ptr<KPBreakpoint> breakpoint = std::make_shared<KPBreakpoint>(0, 0, 0, 0, 0, sum, empty);
    breakpoints.push_back(breakpoint);
    
    /*
     Iterate through each node. Every time we hit a glue node (basically a whitespace)
     evaluate potential breakpoints. The sum variable tracks the total width, stretch,
     and shrink accumulated so far across all nodes.
     */
    for (int i = 0; i < _nodes.size(); i++) {
        std::shared_ptr<KPNode> &node = _nodes[i];
        
        if (node->type == KPNodeType::Box) {
            sum.width += std::dynamic_pointer_cast<KPBox>(node)->width;
        }
        else if (node->type == KPNodeType::Glue) {
            std::shared_ptr<KPGlue> glue = std::dynamic_pointer_cast<KPGlue>(node);

            if (i > 0 && _nodes[i - 1]->type == KPNodeType::Box) {
                findCandidateBreakpoints(node, i, sum, breakpoints);
            }
            
            sum.width   += glue->width;
            sum.stretch += glue->stretch;
            sum.shrink  += glue->shrink;
        }
        else if (node->type == KPNodeType::Penalty &&
                 std::dynamic_pointer_cast<KPPenalty>(node)->penalty != kInfinity) {
            findCandidateBreakpoints(node, i, sum, breakpoints);
        }
    }
    
    std::vector<VROBreakpoint> breaks;
    
    if (!breakpoints.empty()) {
        std::shared_ptr<KPBreakpoint> tmp;
        
        // Find the best active node (the one with the least total demerits.)
        for (std::shared_ptr<KPBreakpoint> &breakpoint : breakpoints) {
            if (!tmp || breakpoint->demerits < tmp->demerits) {
                tmp = breakpoint;
            }
        }
        
        while (tmp) {
            breaks.push_back({ tmp->position, tmp->ratio });
            tmp = tmp->previous;
        }
        
        std::reverse(std::begin(breaks), std::end(breaks));
    }
    
    return breaks;
}

void VROKnuthPlassFormatter::findCandidateBreakpoints(std::shared_ptr<KPNode> &node, int nodeIndex, KPSum &sum,
                                                      std::list<std::shared_ptr<KPBreakpoint>> &breakpoints) const {
    /*
     Iterate through all existing breakpoints. We will try to build a new breakpoint
     at the current node using each existing breakpoint as parent, line by line. 
     
     E.g. first we evaluate all existing breakpoints at line N, then line N + 1, then
     line N + 2, and so on.
     */
    std::list<std::shared_ptr<KPBreakpoint>>::iterator existingBreakpoint_iter = breakpoints.begin();

    while (existingBreakpoint_iter != breakpoints.end()) {
        
        /*
         At maximum there will be four candidates for new breakpoints descending from
         this existing breakpoint. Each corresponds to a different fitness class.
         */
        std::vector<KPBreakpointCandidate> candidates;
        candidates.push_back({ std::numeric_limits<int>::max() });
        candidates.push_back({ std::numeric_limits<int>::max()  });
        candidates.push_back({ std::numeric_limits<int>::max()  });
        candidates.push_back({ std::numeric_limits<int>::max()  });
        
        /*
         This inner loop iterates through all existing breakpoints in the same
         line as existingBreakpoint (including itself). For each, we evaluate the cost
         of creating a new breakpoint at the current node (the node passed into this 
         function), using said existing breakpoint as a parent. If the cost is tolerable, 
         we create a candidate breakpoint.
         */
        while (existingBreakpoint_iter != breakpoints.end()) {
            std::shared_ptr<KPBreakpoint> existingBreakpoint = *existingBreakpoint_iter;
            std::list<std::shared_ptr<KPBreakpoint>>::iterator nextBreakpoint_iter = std::next(existingBreakpoint_iter);
            
            int currentLine = existingBreakpoint->line + 1;
            
            /*
             Compute the width, stretch, and shrink from the existing breakpoint up to the
             current node.
             */
            KPSum sumFromParentToNode = sum - existingBreakpoint->totals;
            if (node->type == KPNodeType::Penalty) {
                sumFromParentToNode.width += std::dynamic_pointer_cast<KPPenalty>(node)->width;
            }
            
            /*
             Compute the stretch or shrink ratio if we were to make the current node a
             breakpoint with the current existing breakpoint as its parent.
             */
            float ratio = computeCost(sumFromParentToNode, existingBreakpoint, currentLine);
            
            /*
             Remove the existing breakpoint entirely if the distance between it and the
             current node becomes too large (exceeding the stretch limit so it becomes
             negative), or when the current node is a forced break.
             */
            if (ratio < -1 ||
                (node->type == KPNodeType::Penalty && std::dynamic_pointer_cast<KPPenalty>(node)->penalty == -kInfinity)) {
                breakpoints.erase(existingBreakpoint_iter);
            }
            
            /*
             If the ratio is within the valid range, then calculate the demerits and record a
             candidate breakpoint.
             */
            if (-1 <= ratio && ratio <= _options.tolerance) {
                
                int demerits = 0;
                float badness = 100 * pow(std::abs(ratio), 3);
                
                if (node->type == KPNodeType::Penalty && std::dynamic_pointer_cast<KPPenalty>(node)->penalty >= 0) {
                    demerits = pow(_options.demerits.line + badness, 2) + pow(std::dynamic_pointer_cast<KPPenalty>(node)->penalty, 2);
                }
                else if (node->type == KPNodeType::Penalty && std::dynamic_pointer_cast<KPPenalty>(node)->penalty != -kInfinity) {
                    demerits = pow(_options.demerits.line + badness, 2) - pow(std::dynamic_pointer_cast<KPPenalty>(node)->penalty, 2);
                }
                else {
                    demerits = pow(_options.demerits.line + badness, 2);
                }
                
                if (node->type == KPNodeType::Penalty && _nodes[existingBreakpoint->position]->type == KPNodeType::Penalty) {
                    demerits += _options.demerits.flagged *
                                std::dynamic_pointer_cast<KPPenalty>(node)->flagged *
                                std::dynamic_pointer_cast<KPPenalty>(_nodes[existingBreakpoint->position])->flagged;
                }
                
                /*
                 Calculate the fitness class for this candidate breakpoint.
                 */
                int currentClass = 0;
                if (ratio < -0.5) {
                    currentClass = 0;
                }
                else if (ratio <= 0.5) {
                    currentClass = 1;
                }
                else if (ratio <= 1) {
                    currentClass = 2;
                }
                else {
                    currentClass = 3;
                }
                
                /*
                 Add a fitness penalty to the demerits if the fitness classes of two adjacent lines
                 differ too much.
                 */
                if (std::abs((float) (currentClass - existingBreakpoint->fitnessClass)) > 1) {
                    demerits += _options.demerits.fitness;
                }
                
                /*
                 Add the total demerits of the parent to get the total demerits of this candidate.
                 */
                demerits += existingBreakpoint->demerits;
                
                /*
                 Only store the best candidate for each fitness class.
                 */
                if (demerits < candidates[currentClass].demerits) {
                    candidates[currentClass] = { existingBreakpoint, demerits, ratio };
                }
            }
            
            existingBreakpoint_iter = nextBreakpoint_iter;
            
            /*
             We break out of the inner loop here in order to process the candidates we've accumulated
             for this line. Then we'll move on to the next loop with the next iteration of the outer
             loop.
             */
            if (existingBreakpoint_iter != breakpoints.end() && (*existingBreakpoint_iter)->line >= currentLine) {
                break;
            }
        }
        
        /*
         Get the sum from the start of the text up to the current node. This will the sum
         used for all the new breakpoints we create out of the current node.
         */
        KPSum nodeSum = computeSum(sum, nodeIndex);
        
        for (int fitnessClass = 0; fitnessClass < candidates.size(); fitnessClass++) {
            KPBreakpointCandidate &candidate = candidates[fitnessClass];
            
            if (candidate.demerits < std::numeric_limits<int>::max()) {
                std::shared_ptr<KPBreakpoint> breakpoint = std::make_shared<KPBreakpoint>(nodeIndex, candidate.demerits, candidate.ratio,
                                                                                          candidate.parent->line + 1,
                                                                                          fitnessClass, nodeSum, candidate.parent);
                if (existingBreakpoint_iter != breakpoints.end()) {
                    breakpoints.insert(existingBreakpoint_iter, breakpoint);
                }
                else {
                    breakpoints.push_back(breakpoint);
                }
            }
        }
    }
}
    
    
    

    
   
